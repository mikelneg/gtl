/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_
#define YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_

#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
#include <exception>
#include <utility>

namespace gtl {
    

template <typename VertexBufferType, typename IndexBufferType, typename KeyType = std::size_t>
class mesh_group { 
    
    struct mesh_group_entry {
        std::size_t vertex_offset, index_offset, vertex_count, index_count, bone_count;
    };

    VertexBufferType vertex_buffer;
    IndexBufferType  index_buffer;
    
    std::vector<std::pair<KeyType,mesh_group_entry>> offset_data;   // needs to be stable

public:

    mesh_group() = default;
    mesh_group(mesh_group&&) = default;           //  movable..
    mesh_group& operator=(mesh_group&&) = delete; //  but not assignable..
    
    mesh_group(KeyType key, VertexBufferType v, IndexBufferType i) 
    {
        add_mesh(std::move(key), std::move(v), std::move(i));
    }

    void add_mesh(KeyType key, VertexBufferType v, IndexBufferType i, std::size_t bone_count) 
    {
        auto it = std::find_if(begin(offset_data), end(offset_data),[&](auto const& entry) { return entry.first == key; });
        if (it != end(offset_data)) throw std::runtime_error{"mesh_group key already inserted..\n"}; 

        offset_data.emplace_back(std::move(key), 
                                 mesh_group_entry{ vertex_buffer.size(), index_buffer.size(), v.size(), i.size(), bone_count });
        
        vertex_buffer.insert(end(vertex_buffer), begin(v), end(v));
        index_buffer.insert(end(index_buffer), begin(i), end(i));
    }

    template <typename F>
    inline void apply(KeyType key, F func) const
    {
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& entry) { return entry.first == key; });
        if (it == end(offset_data)) throw std::runtime_error{"mesh_group key not found..\n"};         
        auto const& e = *it;        
        func(e.second);
    }

    template <typename F>
    inline void apply(F func) const
    {
        for (auto&& e : offset_data) {        
            func(e.second);
        }
    }

    auto mesh_index_for(KeyType key) const { 
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& entry) { return entry.first == key; });
        if (it == end(offset_data)) throw std::runtime_error{"mesh_group key not found..\n"}; 
        return std::distance(begin(offset_data),it);
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
