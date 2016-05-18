#include "gtl/stage.h"

#include <gtl/events.h>
//#include <gtl/scenes.h>

#include <gtl/gtl_window.h>
#include <gtl/d3d_types.h>
#include <gtl/d3d_funcs.h>

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <boost/variant/get.hpp>

#include <vn/boost_visitors.h>

#include <chrono>
#include <future>
#include <functional>
#include <utility>

#include <unordered_map>
#include <cassert>
#include <algorithm>


//#include <gtl/scenes.h> // TODO remove 
//#include <gtl/demo_transition_scene.h>
#include <gtl/command_variant.h>

#include <gtl/tags.h>
#include <gtl/scene.h>

#include <gtl/events.h>


/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {

    namespace {
        
        struct empty_scene {
                        
            //empty_scene(empty_scene const&) = default;
            //empty_scene& operator=(empty_scene const&) = default;
            
            template <typename...Ts>
            constexpr empty_scene(Ts&&...) noexcept {}

            template <typename ...Ts>
            void operator()(Ts const&...) const {}             
        };

    }



stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
    :  // dev_{get_device(swchain)},
       // swchain_{swchain},
       // cqueue_{cqueue_},        
        //synchronizer_{cqueue_, num_buffers-1, (std::max)(0,static_cast<int>(num_buffers)-2)}, // maximum desync value.   
        scene_{empty_scene{}},            
        event_handler_{},
        //scenes_{},  
        //scenes_{},
        //dxgi_pp{},
        //scene_builder_{[&](auto& scene_graph_, auto& yield_, auto& mutex_){ 
         //                   scene_graph_.transition_scene(yield_, get_device(swchain), cqueue_, swchain, mutex_);   
         //             }},
        quit_flag_{false},
        frame_rate_limiter_{std::chrono::milliseconds(9)}, // frame time limit.. 17 == ~60fps, 9 == ~120fps
        draw_params_{draw_queue_, 0, 0.0f, swchain.rtv_heap()}
{            
        assert(num_buffers > 1);    
        frame_state_.store(sig::frame_consumed);   
        //quit_flag_.test_and_set();        
        work_thread_ = std::thread{&stage::work_thread,
                                   this,
                                        get_device(swchain),
                                        std::ref(swchain),
                                        std::ref(cqueue_),
                                        num_buffers                                       
                                   };
}

stage::~stage() 
{
    {
        std::unique_lock<std::mutex> lock{work_mutex_};
        //quit_flag_.clear(std::memory_order_release);
        quit_flag_ = true;        
        frame_state_.store(sig::frame_consumed,std::memory_order_release);            
        cv_.notify_all();    
    }
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
}



namespace { // implementation detail..
    struct resource_object {
        gtl::d3d::direct_command_allocator calloc_;
        gtl::d3d::graphics_command_list clist_before_;
        gtl::d3d::graphics_command_list clist_after_;
        resource_object(gtl::d3d::device& dev) : calloc_{dev}, clist_before_{dev,calloc_}, clist_after_{dev,calloc_} {}
        resource_object(resource_object&&) = default;
        resource_object& operator=(resource_object&&) = default;
    };
        
    template <typename T>
    using id_t = T;            

    template <typename ...Ts, typename ...Qs>    
    static 
    void assign_draw_params(std::tuple<Ts...>& tuple_, Qs&&...qs) {
        tuple_ = {qs...};
    }    
}



void stage::work_thread(gtl::d3d::device dev_, gtl::d3d::swap_chain& swchain_, 
                        gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
                         
{
    std::unique_lock<std::mutex> lock_{work_mutex_};
    
    //
    std::vector<resource_object> buffered_resource_;
    buffered_resource_.reserve(num_buffers);

    for (unsigned i = 0; i < num_buffers; ++i) {
        buffered_resource_.emplace_back(dev_);
    }
    //

    gtl::d3d::synchronization_object synchronizer_{cqueue_, num_buffers-1, 
                                            static_cast<unsigned>((std::max)(0,static_cast<int>(num_buffers)-2))};                                        


    //while(quit_flag_.test_and_set(std::memory_order_acq_rel)) {    
    while(!quit_flag_) {    
        cv_.wait(lock_, [this](){ 
            return frame_state_.load(std::memory_order_acquire) == sig::frame_consumed;             
        });                
        
        //if (!quit_flag_.test_and_set(std::memory_order_acq_rel)) { return; }
        if (quit_flag_) { return; }

        synchronizer_([&buffered_resource_,&cqueue_,&swchain_,this](auto& sync_index) {                        
            assert(value(sync_index) < buffered_resource_.size());
            resource_object& ro_ = buffered_resource_[value(sync_index)];        
            gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
            gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
            gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
            
            auto res1 = calloc->Reset();
            (void)res1;
            auto res2 = clb->Reset(calloc.get(), nullptr);
            (void)res2;
        
            clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              swchain_.get_current_resource(),                                           
                                              D3D12_RESOURCE_STATE_PRESENT,
                                              D3D12_RESOURCE_STATE_RENDER_TARGET));

            clb->Close();

            auto res3 = cla->Reset(calloc.get(), nullptr);
            (void)res3;

            cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              swchain_.get_current_resource(), 
                                              D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                              D3D12_RESOURCE_STATE_PRESENT));            
            cla->Close();
            
            std::vector<ID3D12CommandList*> &v = draw_queue_;

            v.clear();

            v.emplace_back(clb.get());
            
            // TODO rethink this call..

            assign_draw_params(draw_params_, v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap());                

            scene_.send_command(gtl::commands::draw{});   
            // causes scene_ to call draw_callback with a functional like [&](auto&&...ps){ scene_internal_.draw(ps...); }            
            // draw_callback then calls that functional with draw_params_ expanded            

            // // modifying to work with attach_scene ..
            //auto vec = boost::apply_visitor([&](auto& scene){      
            //    return scene.draw(static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); // draw call..
            //},scenes_.current_scene());
            
            //current_scene_.send_command(gtl::commands::draw{});            

            //for (auto&& e : vec) v.emplace_back(e); 
            v.emplace_back(cla.get());    

            //
            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());                                                
            advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready                                   
            frame_state_.store(sig::frame_ready,std::memory_order_release);                            
       },[](){});
    }
}

void stage::present(gtl::d3d::swap_chain& swchain_, gtl::d3d::PresentParameters dxgi_pp)
{          
    frame_rate_limiter_(
        [&swchain_, &dxgi_pp, this](){         
            if (frame_state_.load(std::memory_order_acquire) == sig::frame_ready) {
                swchain_->Present1(0,0,std::addressof(dxgi_pp)); // potentially blocking 
                frame_state_.store(sig::frame_consumed, std::memory_order_release);   
                cv_.notify_one();
            }                
        });    
}

//void stage::event_handler(coro::pull_type& yield)
//{
//}

} // namespace
