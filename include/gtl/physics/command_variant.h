/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef ZSLFWJAFERGWAG_GTL_PHYSICS_COMMAND_VARIANT_H_
#define ZSLFWJAFERGWAG_GTL_PHYSICS_COMMAND_VARIANT_H_

#include <gtl/physics/units.h>
#include <gtl/physics/common_types.h>
#include <gtl/physics/generator_interface.h>

#include <gtl/common/entity_id.h>

#include <vector>
#include <memory>

#include <boost/mpl/begin_end.hpp>
#include <boost/variant.hpp>

namespace gtl {
namespace physics {
namespace commands {
        
    struct static_box {
        position<float> xy_;
        dimensions<float> wh_;
        angle<float> angle_;
        // uint32_t id;
        entity_render_data info_;
    };

    struct dynamic_box {
        position<float> xy_;
        dimensions<float> wh_;
        angle<float> angle_;
        // uint32_t id;
        entity_render_data info_;
    };

    struct dynamic_jointed_boxes {
        std::vector<dynamic_box> boxes_;
        entity_render_data info_;
    };

    struct static_circle {
        float x, y, r, a;
        entity_render_data info_;
    };

    struct destroy_object_implode {
        uint16_t id;
    };

    struct boost_object {
        uint16_t id;
    };

    struct boost_object_vec {
        uint16_t id;
        float x, y;
    };

    struct drive_object_vec {
        uint16_t id;
        float x, y;
    };

    struct polymorphic_generator {
        std::unique_ptr<gtl::physics::generator> generator_;    // HACK could be problematic.. see if the other generator types are sufficient, 
                                                                // or if it makes sense to have a distinct polymorphic_generator type with its own
                                                                // queue given to an implementation..         
    };
    
    using command_mpl_seq_ = typename boost::mpl::list<
                                              static_box, 
                                              dynamic_box, 
                                              dynamic_jointed_boxes, 
                                              static_circle, 
                                              destroy_object_implode, 
                                              boost_object, 
                                              boost_object_vec, 
                                              drive_object_vec,
                                              polymorphic_generator
                                              >::type;    
}

using command_variant = typename boost::make_variant_over<commands::command_mpl_seq_>::type;


}} // namespace
#endif