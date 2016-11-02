/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef WOIFJIWOJEFFE_GTL_ENTITY_RENDER_DATA_H_
#define WOIFJIWOJEFFE_GTL_ENTITY_RENDER_DATA_H_

/*-------------------------------------------------------------
---------------------------------------------------------------*/

#include <cstdint>
#include <type_traits>

namespace gtl {
namespace entity {

    struct render_data { // HACK fix this..

        union {
            uintptr_t data;
            uint16_t arr[4]; // uint16_t <control_point_offset, material_id, entity_id, mesh_id>
        };
        static_assert(sizeof(uintptr_t) == sizeof(uint64_t), "render_data will not be sized properly; must be 64 bits.");

        render_data(uintptr_t i, uint16_t p) noexcept : data{i}
        {
            pack<0>(p);
        }
        explicit render_data(uintptr_t v) noexcept : data{v}
        {
        }
        explicit render_data(uint16_t a, uint16_t b, uint16_t c, uint16_t d) noexcept : arr{a, b, c, d}
        {
        }

        render_data() = default;

        constexpr render_data(render_data const& i) noexcept : data{i.data}
        {
        }

        template <unsigned N>
        render_data& pack(uint16_t i) noexcept
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

        render_data& pack_bone_offset(uint16_t x) noexcept
        {
            return pack<0>(x);
        }
        render_data& pack_material_id(uint16_t x) noexcept
        {
            return pack<1>(x);
        }
        render_data& pack_entity_id(uint16_t x) noexcept
        {
            return pack<2>(x);
        }
        render_data& pack_mesh_id(uint16_t x) noexcept
        {
            return pack<3>(x);
        }

        uintptr_t value() const noexcept
        {
            return data;
        }

        // friend bool operator<(render_data const& lhs, render_data const& rhs) noexcept {
        //    return
        //}

        template <typename T>
        friend T& operator<<(T& str, render_data const& d)
        {
            str << d.data;
            return str;
        }
    };
}
} // namespace
#endif
