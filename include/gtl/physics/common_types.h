/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef URUWOZOBBASFFW_GTL_PHYSICS_COMMON_TYPES_H_
#define URUWOZOBBASFFW_GTL_PHYSICS_COMMON_TYPES_H_

/*-------------------------------------------------------------    
---------------------------------------------------------------*/

#include <cstdint>
#include <vector>

#include <type_traits>

#include <Eigen/Core>

namespace gtl {
namespace physics {

    using entity_transform = Eigen::Matrix4f;

    struct entity_render_data { // HACK fix this..
        
        union {
            uintptr_t data;
            uint16_t arr[4]; // uint16_t <control_point_offset, material_id, entity_id, mesh_id>
        };
                    static_assert(sizeof(uintptr_t) == sizeof(uint64_t), 
                                  "entity_render_data will not be sized properly; must be 64 bits.");
    
        explicit entity_render_data(uintptr_t i, uint16_t p) noexcept : data{i}
        {
            pack<0>(p);
        }
        constexpr explicit entity_render_data(uintptr_t v) noexcept : data{v}
        {
        }
        constexpr explicit entity_render_data(uint16_t a, uint16_t b, uint16_t c, uint16_t d) noexcept : arr{a, b, c, d}
        {
        }
    
        entity_render_data() = default;
    
        constexpr entity_render_data(entity_render_data const& i) noexcept : data{i.data}
        {
        }
    
        template <unsigned N>
        entity_render_data& pack(uint16_t i) noexcept
        {
            static_assert(N <= std::extent<decltype(arr)>{}, "");
            arr[N] = i;
            return *this;
        }
    
        template <unsigned N>
        uint16_t unpack() const noexcept
        {
            static_assert(N <= std::extent<decltype(arr)>{}, "");
            return arr[N];
        }
    
        uint16_t bone_offset() const noexcept
        {
            return unpack<0>();
        }
        uint16_t material_id() const noexcept
        {
            return unpack<1>();
        }
        uint16_t entity_id() const noexcept
        {
            return unpack<2>();
        }
        uint16_t mesh_id() const noexcept
        {
            return unpack<3>();
        }
    
        entity_render_data& pack_bone_offset(uint16_t x) noexcept
        {
            return pack<0>(x);
        }
        entity_render_data& pack_material_id(uint16_t x) noexcept
        {
            return pack<1>(x);
        }
        entity_render_data& pack_entity_id(uint16_t x) noexcept
        {
            return pack<2>(x);
        }
        entity_render_data& pack_mesh_id(uint16_t x) noexcept
        {
            return pack<3>(x);
        }
    
        uintptr_t value() const noexcept
        {
            return data;
        }
    
        // friend bool operator<(entity_render_data const& lhs, entity_render_data const& rhs) noexcept {
        //    return
        //}
    
        template <typename T>
        friend T& operator<<(T& str, entity_render_data const& d)
        {
            str << d.data;
            return str;
        }
    };    

    struct simulation_render_data {
        
        std::vector<entity_render_data> entities_;      
        std::vector<entity_transform> control_points_;  
                                                        
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

    


    //struct entity_data {
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
