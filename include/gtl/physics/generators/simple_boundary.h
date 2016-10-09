/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BHOWIAHJOSIJFAO_GTL_PHYSICS_GENERATORS_SIMPLE_BOUNDARY_H_
#define BHOWIAHJOSIJFAO_GTL_PHYSICS_GENERATORS_SIMPLE_BOUNDARY_H_

/*-------------------------------------------------------------

    class gtl::physics::generators::simple_boundary 

    * simple_boundary is just a square, with dimensions specified in
      meters, with one-meter thick walls.

---------------------------------------------------------------*/

#include <gtl/physics/generator_interface.h>
#include <gtl/physics/units.h>

namespace gtl {
namespace physics {       
namespace generators {
            
    class simple_boundary : public gtl::physics::generator {
        gtl::physics::dimensions<float> dimensions_;
        gtl::physics::position<float> center_;        

    public:        
        simple_boundary(gtl::physics::position<float> center, gtl::physics::dimensions<float> dims)
            : dimensions_(dims),
              center_(center)
        {}

        void apply(b2World&) const final;

        ~simple_boundary() final {}
    };

}}} // namespaces
#endif
