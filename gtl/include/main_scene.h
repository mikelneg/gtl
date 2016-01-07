#ifndef LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_
#define LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scenes
    
    class main_scene;
-----------------------------------------------------------------------------*/

#include <gtl/include/events.h>
#include <gtl/include/keyboard_enum.h>

#include <iostream> 
#include <memory>

namespace gtl {
namespace scenes {

    //class B { public: B() { std::cout << "B()\n"; } B(B const&) { std::cout << "B(B const&)\n"; } ~B() { std::cout << "~B()\n"; } };
        
    class main_scene {
        //std::unique_ptr<B> b;    
    public:
        main_scene() = default;
        //main_scene() : b{std::make_unique<B>()} {}
        //main_scene(main_scene const&) = default;

        void draw(float f) const {
            std::cout << std::fixed << "main_scene(" << f << ")";
        }
        
        template <typename YieldType>
        gtl::event handle_events(YieldType& yield) const {
            namespace ev = gtl::events;
            int count{};
            while (!same_type(yield().get(),ev::exit_immediately{})){                                
                if (same_type(yield.get(),ev::keydown{})){ 
                    if ( boost::get<ev::keydown>( yield.get().value() ).key == gtl::keyboard::Q) {
                        std::cout << "main_scene(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
                        return gtl::events::exit_state{0};
                    } else 
                    if ( boost::get<ev::keydown>( yield.get().value() ).key == gtl::keyboard::K) {
                        std::cout << "main_scene(): k pressed, throwing (none == " << count << ")\n";                                                
                        throw std::runtime_error{__func__};
                        //return 1;
                    } else { std::cout << "main_scene(): some key pressed..\n"; }
                } else if (same_type(yield.get(),ev::none{})) {
                    count++;
                 }
            }            
            return gtl::events::exit_state{0};
        }
    };

}} // namespaces
#endif
