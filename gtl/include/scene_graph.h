#ifndef RIOOEOACAFSDF_GTL_SCENES_SCENE_GRAPH_H_
#define RIOOEOACAFSDF_GTL_SCENES_SCENE_GRAPH_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scene_graph
        
-----------------------------------------------------------------------------*/
#include <iostream> // TODO remove

#include <unordered_map>
#include <utility>
#include <algorithm>


#include <future>
#include <thread>
#include <chrono>

#include <boost/variant.hpp>
#include <vn/include/boost_visitors.h>

#include <gtl/include/scenes.h>
#include <gtl/include/d3d_types.h>

//#include <gtl/include/intro_scene.h>
//#include <gtl/include/main_scene.h>
#include <gtl/include/swirl_effect_transition_scene.h>
#include <gtl/include/demo_transition_scene.h>

namespace gtl {    

    template <typename T, typename F>    
    auto wait_on(T& yield, F fut) {
        while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) { // deferred, ready, timeout                                
            yield();            
        }                           
        return fut.get();  
    };  

    class scene_graph {                                                    

    public:        
        using scene_type = 
            gtl::scenes::scene_variant<//gtl::scenes::intro_scene, 
                                       //gtl::scenes::main_scene,
                                       gtl::scenes::transitions::swirl_effect,
                                       gtl::scenes::transitions::twinkle_effect>; // TODO private

        struct scene_hash {        
            std::size_t operator()(scene_type const& s) const { return boost::apply_visitor(scenes::detail::variant_hash<scene_type>{},s); }
        };

        struct scene_eq {
            bool operator()(scene_type const& lhs, scene_type const& rhs) const { return boost::apply_visitor(vn::visitors::weak_equality{},lhs,rhs); }
        };

        using graph_type = std::unordered_multimap<scene_type, std::pair<gtl::event, scene_type>, scene_hash, scene_eq>;

        graph_type graph_;
        scene_type current_scene_;

        scene_graph() {}
        scene_type& current_scene() { 
            return current_scene_;         
        }        

        template <typename A, typename B, typename C, typename D, typename R>
        void transition_scene(A& yield, B& dev, C& cqueue, D& swapchain, R& rootsig) {            
        //
            using transition_scene = scenes::detail::transition_scene<scene_type>;
            using inv_transition_scene = scenes::detail::inverse_transition_scene<scene_type>;
            auto handle_events_v = vn::make_lambda_visitor<gtl::event>([&](auto& v){ return v.handle_events(yield); });
            auto handle_events = boost::apply_visitor(handle_events_v);//[&](scene_type& s) { return boost::apply_visitor(handle_events_v,s); };
        //
            scene_type& s = current_scene_;             
        //    //s = scenes::transitions::swirl_effect{dev,swchain,cqueue};            
        //    //handle_events(s);
                                
            s = scenes::transitions::twinkle_effect{dev,swapchain,cqueue,rootsig};
            handle_events(s);
        
            while (true) {                                
        
                s = scene_type{transition_scene{std::move(s), scenes::transitions::swirl_effect{dev,swapchain,cqueue,rootsig}, std::chrono::seconds(2)}};
                handle_events(s);
        
                s = boost::get<transition_scene>(s).swap_second(scenes::detail::empty_scene{});                                    
                handle_events(s);            
        
                s = scene_type{inv_transition_scene{std::move(s), scenes::transitions::twinkle_effect{dev,swapchain,cqueue,rootsig}, std::chrono::seconds(2)}};
                handle_events(s);
        
                s = boost::get<inv_transition_scene>(s).swap_second(scenes::detail::empty_scene{});                                    
                handle_events(s);            
            }   
        }
        //
        //    //std::promise<scene_type> prom;
        //    s = boost::get<transition_scene>(s).swap_second(scenes::detail::empty_scene{});
        //    scene_type new_scene{ wait_on(yield, 
        //        std::async(std::launch::async, 
        //            [](){ 
        //                std::cout << "async init...\n";   
        //                scene_type s{scenes::main_scene{}};
        //                std::this_thread::sleep_for(std::chrono::seconds(3));                              
        //                std::cout << "async ready..\n";    
        //                return s;
        //            }))};
        //    
        //    s = scene_type{transition_scene{std::move(s), std::move(new_scene), std::chrono::seconds(2)}};                        
        //    handle_events(s);            
        //
        //    s = boost::get<transition_scene>(s).swap_second(scenes::detail::empty_scene{});                                    
        //    handle_events(s);                                                       

        // works fine, testing..
        //
        //    using transition_scene = scenes::detail::transition_scene<scene_type>;
        //    //auto handle_events_l = [&](auto& s){ return s.handle_events(yield); };
        //    //auto handle_events = boost::apply_visitor(handle_events_l);
        //    auto handle_events_v = vn::make_lambda_visitor<gtl::event>([&](auto& v){ return v.handle_events(yield); });
        //    auto handle_events = boost::apply_visitor(handle_events_v);//[&](scene_type& s) { return boost::apply_visitor(handle_events_v,s); };
        //
        //    scene_type& s = current_scene_; 
        //
        //    s = scene_type{scenes::intro_scene{}};
        //    handle_events(s);
        //
        //    s = scene_type{transition_scene{std::move(s), scenes::transitions::swirl_effect{}, std::chrono::seconds(2)}};
        //    handle_events(s);
        //
        //    //std::promise<scene_type> prom;
        //    s = boost::get<transition_scene>(s).swap_second(scenes::detail::empty_scene{});
        //    scene_type new_scene{ wait_on(yield, 
        //        std::async(std::launch::async, 
        //            [](){ 
        //                std::cout << "async init...\n";   
        //                scene_type s{scenes::main_scene{}};
        //                std::this_thread::sleep_for(std::chrono::seconds(3));                              
        //                std::cout << "async ready..\n";    
        //                return s;
        //            }))};
        //    
        //    s = scene_type{transition_scene{std::move(s), std::move(new_scene), std::chrono::seconds(2)}};                        
        //    handle_events(s);            
        //
        //    s = boost::get<transition_scene>(s).swap_second(scenes::detail::empty_scene{});                                    
        //    handle_events(s);            
        

    };
  
} // namespaces
#endif
