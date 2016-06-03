#ifndef RYUWOFWOABF_GTL_STAGE_H_
#define RYUWOFWOABF_GTL_STAGE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl
    class stage;
-----------------------------------------------------------------------------*/

#include <gtl/d3d_types.h>
#include <gtl/synchronization_object.h>

//#include <boost/coroutine/asymmetric_coroutine.hpp>
//#include <gtl/events.h>

#include <gtl/event_handler.h>
#include <gtl/rate_limiter.h>

#include <gtl/scene.h>
#include <gtl/command_variant.h>
#include <gtl/tags.h>

//#include <vector>
//#include <array>

#include <thread>
#include <tuple>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>

#include <vn/tuple_utilities.h>
#include <vn/work_thread.h>


namespace gtl {

class stage {
    
    struct resource_object {
        gtl::d3d::direct_command_allocator calloc_;
        gtl::d3d::graphics_command_list clist_before_;
        gtl::d3d::graphics_command_list clist_after_;
        resource_object(gtl::d3d::device&& dev) : calloc_{dev}, clist_before_{dev,calloc_}, clist_after_{dev,calloc_} {}        
        resource_object(resource_object&&) = default;
        resource_object& operator=(resource_object&&) = default;
    };
    
    gtl::rate_limiter frame_rate_limiter_;
    std::vector<resource_object> buffered_resource_;
    std::vector<ID3D12CommandList*> draw_queue_;
    gtl::d3d::synchronization_object synchronizer_;
    
    vn::work_thread draw_thread_;
    gtl::coroutine::event_handler event_handler_; 
    
public:
    
    template <typename SceneType>
    stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType&);               

    stage(stage&&) = delete;
    stage& operator=(stage&&) = delete;
    
    void present(gtl::d3d::swap_chain&, DXGI_PRESENT_PARAMETERS);           
    void dispatch_event(gtl::event const& e) { event_handler_.dispatch_event(e); }           
};


template <typename SceneType>
stage::stage(gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType& scene_)
    :   frame_rate_limiter_{std::chrono::milliseconds(9)},
        buffered_resource_{[&](){
            std::vector<resource_object> tmp; 
            for (unsigned i = 0; i < num_buffers; ++i) {
                tmp.emplace_back(get_device(swchain_));
            }
            return tmp;
        }()},    
        synchronizer_{cqueue_, num_buffers-1, static_cast<unsigned>((std::max)(0,static_cast<int>(num_buffers)-2))},
        draw_thread_{[&,this](auto& state_){ 
                synchronizer_([&](auto& sync_index) {                        
                    assert(value(sync_index) < buffered_resource_.size());
                    resource_object& ro_ = buffered_resource_[value(sync_index)];        
                    gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
                    gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
                    gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
                    
                    calloc->Reset();
                    
                    clb->Reset(calloc.get(), nullptr);            
        
                    clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                                      swchain_.get_current_resource(),                                           
                                                      D3D12_RESOURCE_STATE_PRESENT,
                                                      D3D12_RESOURCE_STATE_RENDER_TARGET));

                    clb->Close();
                    cla->Reset(calloc.get(), nullptr);            

                    cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                                      swchain_.get_current_resource(), 
                                                      D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                                      D3D12_RESOURCE_STATE_PRESENT));            
                    cla->Close();                                       

                    draw_queue_.clear();
                    draw_queue_.emplace_back(clb.get());                    

                    // stage_.draw_callback(...); 
                    scene_.draw_callback([&](auto f) { f(draw_queue_, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); });

                    draw_queue_.emplace_back(cla.get());    

                    //            
                    cqueue_->ExecuteCommandLists(static_cast<unsigned>(draw_queue_.size()),draw_queue_.data());                                                
                    synchronous_advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready                                   
                    //frame_state_.store(sig::frame_ready,std::memory_order_release);                            
                    
                    available(state_);
                    },[](){});
        },[](){}},        
        event_handler_{[&](auto& yield){                        
            auto callback_handler_ = 
            vn::make_composite_function(
                [&](gtl::commands::get_swap_chain, auto&& f) 
                {
                    f(swchain_);            
                },
                [&](gtl::commands::get_some_resource, auto&& f) {
                    //std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
                    f([](){ std::cout << "look at me, all fancy..\n"; });
                }
            );
            
            scene_.handle_events(callback_handler_,yield);
        }}
{            
        assert(num_buffers > 1);                            
        event_handler_.dispatch_event(gtl::events::none{});
}
    

/*
class stage {
    
    //gtl::scene<gtl::command_variant const&> scene_;
    gtl::coroutine::event_handler event_handler_; 

    using pull_type = decltype(event_handler_)::pull_type;
    using push_type = decltype(event_handler_)::push_type;

public:
    enum class sig {
        frame_consumed,
        frame_ready,        
    };
private:
    std::thread draw_thread_thread_;
    std::mutex draw_thread_mutex_;
    std::condition_variable cv_;    
    bool quit_flag_; // the mutex guards this, so going with non-atomic
    std::atomic<sig> frame_state_;            

    gtl::rate_limiter frame_rate_limiter_;


    template <typename SceneType>
    void draw_thread_thread(SceneType const& scene_, gtl::d3d::device, gtl::d3d::swap_chain&, gtl::d3d::command_queue&, unsigned);    

public:
    
    template <typename SceneType>
    stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType&&);
    
        
    ~stage();

    stage(stage&&) = delete;
    stage& operator=(stage&&) = delete;
    
    void present(gtl::d3d::swap_chain&,gtl::d3d::PresentParameters);           
    void dispatch_event(gtl::event const& e) { event_handler_.dispatch_event(e); }   
    
    template <typename F>
    void replace_event_handler(F func) { event_handler_.replace_handler(std::move(func)); }        
};


template <typename SceneType>
stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType&& scene_)
    :   event_handler_{},
        quit_flag_{false},
        frame_rate_limiter_{std::chrono::milliseconds(9)}
{            
        assert(num_buffers > 1);    
        frame_state_.store(sig::frame_consumed);   
        
        event_handler_.exchange_handler(
        [&,main_scene_=std::move(scene_)](auto& yield){                        
            auto callback_handler_ = 
            vn::make_composite_function(
                [&](gtl::commands::get_swap_chain, auto&& f) 
                {
                    f(swchain_);            
                },
                [&](gtl::commands::get_some_resource, auto&& f) {
                    std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
                    f([](){ std::cout << "look at me, all fancy..\n"; });
                }
            );

            draw_thread_thread_ = std::thread{&stage::draw_thread_thread<SceneType>,
                                        this,
                                        std::cref(main_scene_),
                                        get_device(swchain),
                                        std::ref(swchain),
                                        std::ref(cqueue_),
                                        num_buffers                                        
                                   };

            main_scene_.handle_events(callback_handler_,yield);
        });

        event_handler_.dispatch_event(gtl::events::none{});
        //draw_thread_thread_ = std::thread{&stage::draw_thread_thread,
        //                           this,
        //                                get_device(swchain),
        //                                std::ref(swchain),
        //                                std::ref(cqueue_),
        //                                num_buffers                                       
        //                           };
}


    struct resource_object {
        gtl::d3d::direct_command_allocator calloc_;
        gtl::d3d::graphics_command_list clist_before_;
        gtl::d3d::graphics_command_list clist_after_;
        resource_object(gtl::d3d::device& dev) : calloc_{dev}, clist_before_{dev,calloc_}, clist_after_{dev,calloc_} {}
        resource_object(resource_object&&) = default;
        resource_object& operator=(resource_object&&) = default;
    };

template <typename SceneType>
void stage::draw_thread_thread(SceneType const& scene_, gtl::d3d::device dev_, gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
                         
{
    using sig = stage::sig;
    //

    std::unique_lock<std::mutex> lock_{draw_thread_mutex_};        
    std::vector<resource_object> buffered_resource_;    

    for (unsigned i = 0; i < num_buffers; ++i) {
        buffered_resource_.emplace_back(dev_);
    }


    gtl::d3d::synchronization_object synchronizer_{cqueue_, num_buffers-1, 
                                            static_cast<unsigned>((std::max)(0,static_cast<int>(num_buffers)-2))};                                        

    std::vector<ID3D12CommandList*> draw_queue_;

    //while(quit_flag_.test_and_set(std::memory_order_acq_rel)) {    
    while(!quit_flag_) {    
        cv_.wait(lock_, [&](){ 
            return frame_state_.load(std::memory_order_acquire) == sig::frame_consumed;             
        });                
        
        //if (!quit_flag_.test_and_set(std::memory_order_acq_rel)) { return; }
        if (quit_flag_) { return; }

        //synchronizer_([&buffered_resource_,&cqueue_,&swchain_,&main_scene_](auto& sync_index) {                        
        synchronizer_([&](auto& sync_index) {                        
            assert(value(sync_index) < buffered_resource_.size());
            resource_object& ro_ = buffered_resource_[value(sync_index)];        
            gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
            gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
            gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
            
            calloc->Reset();
            
            clb->Reset(calloc.get(), nullptr);            
        
            clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              swchain_.get_current_resource(),                                           
                                              D3D12_RESOURCE_STATE_PRESENT,
                                              D3D12_RESOURCE_STATE_RENDER_TARGET));

            clb->Close();
            cla->Reset(calloc.get(), nullptr);            

            cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              swchain_.get_current_resource(), 
                                              D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                              D3D12_RESOURCE_STATE_PRESENT));            
            cla->Close();
            
            std::vector<ID3D12CommandList*> &v = draw_queue_;

            v.clear();

            v.emplace_back(clb.get());
            
            
            scene_.draw_callback([&](auto f) { f(v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); });

            v.emplace_back(cla.get());    

            //            
            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());                                                
            synchronous_advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready                                   
            frame_state_.store(sig::frame_ready,std::memory_order_release);                            
       },[](){});
    }
}
*/

  
} // namespace
#endif           
  
  
  
