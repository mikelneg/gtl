#include "../include/stage.h"

#include <gtl/include/events.h>
#include <gtl/include/scenes.h>

#include <gtl/include/gtl_window.h>
#include <gtl/include/d3d_types.h>
#include <gtl/include/d3d_funcs.h>

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <boost/variant/get.hpp>

#include <vn/include/boost_visitors.h>

#include <chrono>
#include <future>

#include <unordered_map>
#include <cassert>


#include <gtl/include/scenes.h> // TODO remove 
#include <gtl/include/demo_transition_scene.h>



/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {


stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, unsigned max_desync)       
    :   dev_{get_device_from(swchain)},
        swchain_{swchain},
        cqueue_{cqueue_},
        num_buffers_{num_buffers},
        synchronizer_{cqueue_,num_buffers-1,max_desync},   
        scenes_{},
        root_sig_{dev_,gtl::d3d::dummy_rootsig_3()}
        //calloc_{{{dev_},{dev_},{dev_}}},
        //clist_before_{{{dev_, calloc_[0]},{dev_, calloc_[1]},{dev_, calloc_[2]}}},
        //clist_after_{{{dev_, calloc_[0]},{dev_, calloc_[1]},{dev_, calloc_[2]}}}
{
        buffered_resource_.reserve(num_buffers);

        for (unsigned i = 0; i < num_buffers; ++i) {
            buffered_resource_.emplace_back(dev_);
        }

        assert(num_buffers > 1);    
}



void stage::draw(float f)
{      
        
    auto fu = [&](auto idx) {                
        assert(idx < buffered_resource_.size());
        resource_object& ro_ = buffered_resource_[idx];        
        gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
        gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
        gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
        
        auto res1 = calloc->Reset();
        auto res2 = clb->Reset(calloc.get(), nullptr);                
    
        clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          swchain_.get_current_resource(),                                           
                                          D3D12_RESOURCE_STATE_PRESENT,
                                          D3D12_RESOURCE_STATE_RENDER_TARGET));

        clb->Close();

        auto res3 = cla->Reset(calloc.get(), nullptr);

        cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          swchain_.get_current_resource(), 
                                          D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                          D3D12_RESOURCE_STATE_PRESENT));
        
        cla->Close();

        std::vector<ID3D12CommandList*> v;
        v.emplace_back(clb.get());
        auto vec = boost::apply_visitor([&](auto& scene){ return scene.draw(static_cast<int>(idx), f, swchain_.rtv_heap()); }, 
                                                    scenes_.current_scene());
        for (auto&& e : vec) v.emplace_back(e); 
        v.emplace_back(cla.get());    

        //std::initializer_list<ID3D12CommandList*> clists{clist_.get()};                                         
        //if (WaitForSingleObject(swchain_->GetFrameLatencyWaitableObject(),0) == WAIT_OBJECT_0) {        
            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());
            swchain_->Present(0,0);               
            return true;
        //} 
        //    return false;
    };

    synchronizer_(fu,[](){}); // TODO this only synchronizes with the command queue; it doesn't 
                              // guarantee that frames have been presented and that resources
                              // are free to reuse --                                                                                                     
 
    //wait_for_gpu(dev_,cqueue_);    
    //},[](){});    
}
 

void stage::handle_events(coro::pull_type& yield)
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
        
    scenes_.transition_scene(yield, dev_, cqueue_, swchain_, root_sig_);

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


stage::coro::push_type stage::make_event_handler() 
{
    return coro::push_type{
        [this](coro::pull_type& yield) mutable
        {
            this->handle_events(yield);                                                        
        }};        
}


} // namespace