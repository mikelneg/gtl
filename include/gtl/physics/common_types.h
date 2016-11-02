/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef URUWOZOBBASFFW_GTL_PHYSICS_COMMON_TYPES_H_
#define URUWOZOBBASFFW_GTL_PHYSICS_COMMON_TYPES_H_

/*-------------------------------------------------------------
---------------------------------------------------------------*/

//#include <cstdint>
//#include <vector>
//#include <type_traits>
//#include <gtl/entity/render_data.h>
#include <Eigen/Core>

namespace gtl {
namespace physics {

    using Matrix4f = Eigen::Matrix<float, 4, 4, Eigen::RowMajor>;

    // Moved .... using entity_transform = Eigen::Matrix4f;
    //
    // Moved .... struct simulation_render_data {
    //
    //    std::vector<entity::render_data> entities_;
    //    std::vector<entity_transform> control_points_;
    //
    //    // "entities" are packs of data for the renderer, currently stored as
    //    //      <control_point_offset, material_id, entity_id, mesh_id>
    //    //
    //    // control_points are just transforms; the <control_point_offset> element of the entity
    //    // refers to an index in this vector, and the <mesh_id> element (eventually) determines
    //    // the number of (sequential) tranforms that are used in rendering the entity
    //    //
    //
    //    friend void swap(simulation_render_data& lhs, simulation_render_data& rhs)
    //    {
    //        using std::swap;
    //        swap(lhs.entities_, rhs.entities_);
    //        swap(lhs.control_points_, rhs.control_points_);
    //    }
    //
    //    void clear() {
    //        entities_.clear();
    //        control_points_.clear();
    //    }
    //};
    //
    //
    //
    //
    // Moved ... struct entity_data {
    //
    //    // Eigen::Vector4f xywh_;
    //    // Eigen::Vector4f rgb_angle_;
    //
    //    // uint32_t bone_offset_;
    //    // uint32_t bone_count_;
    //
    //    union {
    //        uintptr_t entity_data_;
    //        uint16_t arr[4];
    //    };
    //
    //    // uintptr_t entity_data_; // 64 bit: <mesh_id, entity_id, material_id, EMPTY>
    //    // ..
    //    // 64 bit: <mesh_id, entity_id, material_id, ?? something_physics_consumes>
    //    // 64 bit: <mesh_id, entity_id, material_id, ?? something_rendering_consumes>
    //    // EntityData...
    //    // uint32_t mesh_id_;
    //    // uint32_t id_;
    //    // etc..
    //
    //    template <int N>
    //    uintptr_t pack(uint16_t v) noexcept
    //    {
    //        return entity_data_
    //    }
    //
    //    friend bool operator<(entity_data const& lhs, entity_data const& rhs)
    //    {
    //        return lhs.entity_data_ < rhs.entity_data_;
    //    }
    //};
}
} // namespace
#endif
