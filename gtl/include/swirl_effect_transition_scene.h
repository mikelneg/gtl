#ifndef UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_
#define UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scenes::transitions::
    
    class swirl_effect;
-----------------------------------------------------------------------------*/

#include <gtl/include/events.h>
#include <gtl/include/keyboard_enum.h>

#include <iostream> 

namespace gtl {
namespace scenes {
namespace transitions {

    class swirl_effect {
    public:
        swirl_effect() = default;        

        void draw(float f) const {
            std::cout << std::fixed << "swirl_effect(" << f << ")";
        }
        
        template <typename YieldType>
        gtl::event handle_events(YieldType& yield) const {
            // TODO this will probably never be called.. 
            return gtl::events::exit_state{0};
        }
    };

}}} // namespaces
#endif
