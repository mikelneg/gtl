#include "gtl/stage.h"

#include <gtl/events.h>
#include <gtl/scenes.h>

#include <gtl/gtl_window.h>
#include <gtl/d3d_types.h>
#include <gtl/d3d_funcs.h>

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <boost/variant/get.hpp>

#include <vn/boost_visitors.h>

#include <chrono>
#include <future>
#include <functional>

#include <unordered_map>
#include <cassert>
#include <algorithm>


#include <gtl/scenes.h> // TODO remove 
#include <gtl/demo_transition_scene.h>


/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {


stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
    :   dev_{get_device(swchain)},
        swchain_{swchain},
        cqueue_{cqueue_},        
        synchronizer_{cqueue_, num_buffers-1, (std::max)(0,static_cast<int>(num_buffers)-2)}, // maximum desync value.   
        event_handler_{[this](auto& yield){ this->event_handler(yield); }},
        scenes_{},        
        dxgi_pp{},
        quit_flag_{false},
        frame_rate_limiter_{std::chrono::milliseconds(9)} // frame time limit.. 17 == ~60fps, 9 == ~120fps
{        
        assert(num_buffers > 1);    
        frame_state_.store(sig::frame_consumed);   
        //quit_flag_.test_and_set();        
        work_thread_ = std::thread{&stage::work_thread,this,num_buffers};
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
}

void stage::work_thread(unsigned num_buffers) 
{
    std::unique_lock<std::mutex> lock_{work_mutex_};
    
    //
    std::vector<resource_object> buffered_resource_;
    buffered_resource_.reserve(num_buffers);

    for (unsigned i = 0; i < num_buffers; ++i) {
        buffered_resource_.emplace_back(dev_);
    }
    //

    //while(quit_flag_.test_and_set(std::memory_order_acq_rel)) {    
    while(!quit_flag_) {    
        cv_.wait(lock_, [this](){ 
            return frame_state_.load(std::memory_order_acquire) == sig::frame_consumed;             
        });                
        
        //if (!quit_flag_.test_and_set(std::memory_order_acq_rel)) { return; }
        if (quit_flag_) { return; }

        synchronizer_([&buffered_resource_,this](auto& sync_index) {                        
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

            std::vector<ID3D12CommandList*> v;
            v.emplace_back(clb.get());
            
            auto vec = boost::apply_visitor([&](auto& scene){ 
                return scene.draw(static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); // draw call..
            },scenes_.current_scene());

            for (auto&& e : vec) v.emplace_back(e); 
            v.emplace_back(cla.get());    

            //
            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());                                                
            advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready                                   
            frame_state_.store(sig::frame_ready,std::memory_order_release);                            
       },[](){});
    }
}

void stage::present()
{          
    frame_rate_limiter_(
        [this](){         
            if (frame_state_.load(std::memory_order_acquire) == sig::frame_ready) {
                swchain_->Present1(0,0,std::addressof(dxgi_pp)); // potentially blocking 
                frame_state_.store(sig::frame_consumed, std::memory_order_release);   
                cv_.notify_one();
            }                
        });    
}

void stage::event_handler(coro::pull_type& yield)
{
    //using namespace ::gtl::events::event_types;            
    //using transition_scene = scenes::detail::transition_scene<scene_graph::scene_type>;
    //using std::swap;                
    //    
    //auto visit = vn::make_lambda_visitor<gtl::event>([&](auto& v){ return v.handle_events(yield); });
    //
    //scenes_.current_scene() = scenes::intro_scene{};
    //
    //while (!same_type(yield.get(),gtl::events::exit_immediately{})){    
    //    apply_visitor(visit,scenes_.current_scene());                 
    //    scenes_.current_scene() = scene_graph::scene_type{transition_scene{std::move(scenes_.current_scene()), 
    //        scene_graph::scene_type{scenes::main_scene{}}, std::chrono::milliseconds(3000)}};
    //    apply_visitor(visit,scenes_.current_scene());
    //    //if (!state_manager(yield, result)) { return; }                
    //    scenes_.current_scene() = boost::get<transition_scene>(scenes_.current_scene()).swap_second(scenes::detail::empty_scene{});
    //    apply_visitor(visit,scenes_.current_scene());
    //}

    //scenes_.current_scene() = scenes::transitions::twinkle_effect{dev_,cqueue_,gtl::d3d::dummy_rootsig_1()};

    scenes_.transition_scene(yield, dev_, cqueue_, swchain_, work_mutex_);

    //for (;;) {
        //std::cout << "beginning stage handler..\n";  
        ////state_v transit = transition_h{std::move(scene_), state_v{state_B{}}};
        //apply_visitor(visit,current_scene_);        
        //current_scene_ = transition_scene{std::move(current_scene_), 
        //    gtl::scene{main_scene{}}, std::chrono::milliseconds(2000)}; //swap(scene_, transit);
        //apply_visitor(visit,current_scene_);        
        //current_scene_ = std::move(boost::get<main_scene>(boost::get<transition_scene>(current_scene_).second()));
        //apply_visitor(visit,current_scene_);        
        //scene_ = empty_scene{std::move(boost::get<state_B>(boost::get<transition_h>(scene_).s2))};
        //swap(boost::get<transition_h>(scene_).s2, scene_);                        
        //to state_manager = [&](evco::pull_type& yield, int result) {
        //      using boost::get;      
        //      if (result == 1) { 
        //          std::cout << "state_manager swapping states..\n";  
        //          //state_v transit = transition_h{std::move(stayyt), state_v{state_B{}}};
        //          using std::swap;                
        //          stayyt = transition_h{std::move(stayyt), state_v{state_B{}}}; //swap(stayyt, transit);
        //          auto visit = vn::make_lambda_visitor([&](auto& v){ v(yield); }, [](boost::blank){});
        //          boost::apply_visitor(visit,stayyt); // drop to stayyt..                                  
        //          stayyt = state_B{std::move(boost::get<state_B>(boost::get<transition_h>(stayyt).s2))};
        //          //swap(boost::get<transition_h>(stayyt).s2, stayyt);
        //      }
        //      if (result == 2) { std::cout << "state_manager bonking out..\n"; return false; }
        //      return true;    
    //}
}


//stage::coro::push_type stage::make_event_handler(std::function<void()> cleanup_) 
//{
//    return stage::coro::push_type{
//        [cleanup_=std::move(cleanup_),this](stage::coro::pull_type& yield) mutable
//        {
//            try {
//                this->handle_events(yield);                
//                cleanup_();
//                //for (;;) while (yield) { yield(); } 
//            } catch (std::exception& e) { std::cout << "exception caught... " << e.what() << "\n"; }
//        }};        
//}


} // namespace
