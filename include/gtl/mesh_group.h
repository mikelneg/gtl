/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_
#define YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_

#include <algorithm>
#include <iterator>
#include <vector>

namespace gtl {

template <typename V, typename I, typename K>
struct mesh_group { // HACK hackish..

    struct mesh_data {
        K key;
        size_t vertex_offset, index_offset, vertex_count, index_count, bone_count;
    };

    V vertex_buffer;
    I index_buffer;
    std::vector<mesh_data> offset_data;

    mesh_group() = default;
    mesh_group(mesh_group&&) = default;           //  movable..
    mesh_group& operator=(mesh_group&&) = delete; //  but not assignable..

    mesh_group(K key, V v, I i)
    {
        add_mesh(std::move(key), std::move(v), std::move(i));
    }

    void add_mesh(K key, V v, I i, size_t bone_count = 0)
    {
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& entry) { return entry.key == key; });
        if (it != end(offset_data))
            return; // already contains key..

        offset_data.emplace_back(
            mesh_data{std::move(key), vertex_buffer.size(), index_buffer.size(), v.size(), i.size(), bone_count});
        vertex_buffer.insert(end(vertex_buffer), begin(v), end(v));
        index_buffer.insert(end(index_buffer), begin(i), end(i));
    }

    template <typename F>
    inline void apply(K key, F func) const
    {
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& entry) { return entry.key == key; });
        if (it == end(offset_data))
            throw std::out_of_range{__func__}; // key not found..
        auto const& e = *it;
        func(e.vertex_offset, e.index_offset, e.vertex_count, e.index_count, e.bone_count);
    }

    template <typename F>
    inline void apply(F func) const
    {
        for (auto&& e : offset_data)
        {
            func(e.key, e.vertex_offset, e.index_offset, e.vertex_count, e.index_count, e.bone_count);
        }
    }

    auto vertex_data()
    {
        return vertex_buffer.data();
    }
    auto index_data()
    {
        return index_buffer.data();
    }

    size_t vertex_count() const
    {
        return vertex_buffer.size();
    }
    size_t index_count() const
    {
        return index_buffer.size();
    }
    size_t mesh_count() const
    {
        return offset_data.size();
    }
};

} // namespace
#endif
