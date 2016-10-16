/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef OBQPQEIWOFWG_GTL_PHYSICS_SIMULATION_RENDER_DATA_H_
#define OBQPQEIWOFWG_GTL_PHYSICS_SIMULATION_RENDER_DATA_H_

/*-------------------------------------------------------------    
---------------------------------------------------------------*/

#include <cstdint>
#include <vector>

#include <type_traits>

#include <gtl/entity/render_data.h>

#include <Eigen/Core>

namespace gtl {
namespace physics {

    using entity_transform = Eigen::Matrix4f;

    struct simulation_render_data {
        
        std::vector<entity_transform, Eigen::aligned_allocator<entity_transform>> control_points_;  
        std::vector<entity::render_data> entities_;              
                                                        
        // "entities" are packs of data for the renderer, currently stored as 
        //      <control_point_offset, material_id, entity_id, mesh_id>
        //
        // control_points are just transforms; the <control_point_offset> element of the entity
        // refers to an index in this vector, and the <mesh_id> element (eventually) determines
        // the number of (sequential) tranforms that are used in rendering the entity        
        // 
    
        friend void swap(simulation_render_data& lhs, simulation_render_data& rhs)
        {
            using std::swap;
            swap(lhs.entities_, rhs.entities_);
            swap(lhs.control_points_, rhs.control_points_);
        }
    
        void clear() {
            entities_.clear();
            control_points_.clear();
        }
    };
}
} // namespace
#endif
