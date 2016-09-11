/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_
#define RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_

#include <atomic>
#include <thread>
#include <type_traits>
#include <vector>

#include <gtl/swap_vector.h>
#include <vn/swap_object.h>

#include <Eigen/Core>

#include <boost/units/base_units/angle/radian.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/plane_angle.hpp>

#include <boost/variant.hpp>

#include <gtl/physics_units.h>

namespace gtl {

struct EntityInfo {
    // Eigen::Vector4f xywh_;
    // Eigen::Vector4f rgb_angle_;

    // uint32_t bone_offset_;
    // uint32_t bone_count_;

    union {
        uintptr_t entity_data_;
        uint16_t arr[4];
    };

    // uintptr_t entity_data_; // 64 bit: <mesh_id, entity_id, material_id, EMPTY>
    // ..
    // 64 bit: <mesh_id, entity_id, material_id, ?? something_physics_consumes>
    // 64 bit: <mesh_id, entity_id, material_id, ?? something_rendering_consumes>
    // EntityData...
    // uint32_t mesh_id_;
    // uint32_t id_;
    // etc..

    template <int N>
    uintptr_t pack(uint16_t v) noexcept
    {
        return entity_data_
    }

    friend bool operator<(EntityInfo const& lhs, EntityInfo const& rhs)
    {
        return lhs.entity_data_ < rhs.entity_data_;
    }
};

struct InstanceInfo { // HACK hackish..
    union {
        uintptr_t data;
        uint16_t arr[4]; // uint16_t <bone_offset, material_id, entity_id, mesh_id>
    };

    explicit InstanceInfo(uintptr_t i, uint16_t p) noexcept : data{i}
    {
        pack<0>(p);
    }
    constexpr explicit InstanceInfo(uintptr_t v) noexcept : data{v}
    {
    }
    constexpr explicit InstanceInfo(uint16_t a, uint16_t b, uint16_t c, uint16_t d) noexcept : arr{a, b, c, d}
    {
    }

    InstanceInfo() = default;

    constexpr InstanceInfo(InstanceInfo const& i) noexcept : data{i.data}
    {
    }

    template <unsigned N>
    InstanceInfo& pack(uint16_t i) noexcept
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

    InstanceInfo& pack_bone_offset(uint16_t x) noexcept
    {
        return pack<0>(x);
    }
    InstanceInfo& pack_material_id(uint16_t x) noexcept
    {
        return pack<1>(x);
    }
    InstanceInfo& pack_entity_id(uint16_t x) noexcept
    {
        return pack<2>(x);
    }
    InstanceInfo& pack_mesh_id(uint16_t x) noexcept
    {
        return pack<3>(x);
    }

    uintptr_t value() const noexcept
    {
        return data;
    }

    // friend bool operator<(InstanceInfo const& lhs, InstanceInfo const& rhs) noexcept {
    //    return
    //}

    friend std::ostream& operator<<(std::ostream& str, InstanceInfo const& d)
    {
        str << d.data;
        return str;
    }
};

static_assert(sizeof(EntityInfo) == sizeof(uint64_t), "EntityInfo not sized properly, should be 64 bits..");
static_assert(sizeof(InstanceInfo) == sizeof(uint64_t), "InstanceInfo not sized properly, should be 64 bits..");

namespace physics {

    namespace generators {

        struct static_box {
            position<float> xy_;
            dimensions<float> wh_;
            angle<float> angle_;
            // uint32_t id;
            InstanceInfo info_;
        };

        struct dynamic_box {
            position<float> xy_;
            dimensions<float> wh_;
            angle<float> angle_;
            // uint32_t id;
            InstanceInfo info_;
        };

        struct dynamic_jointed_boxes {
            std::vector<dynamic_box> boxes_;
            InstanceInfo info_;
        };

        struct static_circle {
            float x, y, r, a;
            InstanceInfo info_;
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
    }

    using generator
        = boost::variant<generators::static_box, generators::dynamic_box, generators::dynamic_jointed_boxes,
                         generators::static_circle, generators::destroy_object_implode, generators::boost_object,
                         generators::boost_object_vec, generators::drive_object_vec>;
}

struct render_data {
    std::vector<InstanceInfo> entities_;
    std::vector<Eigen::Matrix4f> bones_;

    friend void swap(render_data& lhs, render_data& rhs)
    {
        using std::swap;
        swap(lhs.entities_, rhs.entities_);
        swap(lhs.bones_, rhs.bones_);
    }
};

class physics_simulation {
    using entity_type = InstanceInfo;

    vn::swap_object<render_data> render_data_;

    std::atomic_flag quit_;
    std::thread thread_;

public:
    physics_simulation(gtl::swap_vector<gtl::physics::generator>&);

    bool extract_render_data(render_data& c)
    {
        return render_data_.swap_out(c);
    }

    ~physics_simulation()
    {
        quit_.clear();
        thread_.join();
    }
};

} // namespace
#endif