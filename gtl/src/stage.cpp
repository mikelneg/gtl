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



/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {


stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, unsigned max_desync)       
    :   swchain_{swchain},
        cqueue_{cqueue_},
        sync_{cqueue_,gtl::d3d::fence{get_device_from(swchain)},num_buffers-1,max_desync},
        num_buffers_{num_buffers},
        scenes_{}
{
        assert(num_buffers > 1);    
}



void stage::draw(float f)
{
    boost::apply_visitor([&](auto& scene){ scene.draw(f); }, scenes_.current_scene());
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
      
    scenes_.transition_scene(yield, gtl::d3d::get_device_from(swchain_), cqueue_, swchain_ );

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