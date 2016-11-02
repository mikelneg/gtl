/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UWOIBOASIFAEFEG_GTL_PHYSICS_SIMULATION_INTERFACE_H_
#define UWOIBOASIFAEFEG_GTL_PHYSICS_SIMULATION_INTERFACE_H_

/*-------------------------------------------------------------

---------------------------------------------------------------*/

#include <gtl/physics/simulation_render_data.h>

namespace gtl {
namespace physics {

    class simulation {
    public:
        virtual bool extract_render_data(simulation_render_data&) = 0; // returns true if extract succeeds
        virtual ~simulation()
        {
        }
    };
}
} // namespaces
#endif
