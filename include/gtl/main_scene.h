#ifndef LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_
#define LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scenes
    
    class main_scene;
-----------------------------------------------------------------------------*/

#include <gtl/events.h>
#include <gtl/keyboard_enum.h>

#include <gtl/swirl_effect_transition_scene.h>
#include <gtl/command_variant.h>

namespace gtl {
namespace scenes {


    class main_scene {

        gtl::scenes::transitions::swirl_effect swirl_effect_;

    public:        
        
        template <typename ...Ts>
        main_scene(Ts&&...ts) 
            : swirl_effect_{std::forward<Ts>(ts)...}
        {}

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const {
            func([&](auto&&...ps){ swirl_effect_.draw(std::forward<decltype(ps)>(ps)...); });
        }
        
        template <typename ResourceManager, typename YieldType>
        gtl::event handle_events(ResourceManager& resource_callback_, YieldType& yield) const {
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;
            int count{};
            std::cout << "swirl_effect event handler entered..\n"; 
            while (!same_type(yield().get(),ev::exit_immediately{})){                   
                if (same_type(yield.get(),ev::keydown{})){ 
                    
                    switch( boost::get<ev::keydown>( yield.get().value() ).key ) {
                        case k::Escape : std::cout << "swirl_effect(): escape pressed, exiting all..\n"; 
                                         return gtl::events::exit_all{}; break;
                        case k::Q : std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
                                    return gtl::events::exit_state{0}; break;
                        case k::K : std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";                                                
                                    throw std::runtime_error{__func__}; break;                    
                        case k::R : std::cout << "swirl_effect() : r pressed, resizing swapchain..\n"; 
                                    //resource_callback_(gtl::commands::get_swap_chain{},
                                    //                   [](auto& swchain_){ swchain_.resize(100,100); });                                                                        
                                    resource_callback_(gtl::commands::get_some_resource{},[](auto& r) { r(); });
                                    break;
                        default : std::cout << "swirl_effect() : unknown key pressed\n"; 
                    }
                                   
                } else if (same_type(yield.get(),ev::none{})) {
                    count++;                
                } else if (same_type(yield.get(),ev::mouse_at{})) {
                    swirl_effect_.set_mouse_coords(boost::get<ev::mouse_at>(yield.get().value()).coord);                    
                }
            }            
            return gtl::events::exit_state{0};
        }
    };

}} // namespaces
#endif
