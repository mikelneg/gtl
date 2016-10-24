/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_
#define YYRWIAIFZVVBASDFASDF_GTL_MESH_GROUP_H_

#include <algorithm>
#include <vector>
#include <iterator>
//#include <iostream>
#include <exception>
#include <utility>

#include <gtl/common_mesh.h>


namespace gtl {
    
template <typename V, typename I, typename K = std::size_t>
class mesh_group { 
    
    struct entry {
        unsigned vertex_offset, index_offset, 
                 vertex_count, index_count, 
                 weights_per_vertex;
    };

    std::vector<V> vertex_buffer;
    std::vector<I> index_buffer;    
    std::vector<std::pair<K,entry>> offset_data;    // we use vector<> here rather than something like flat_map<> for offset_data because we need to preserve insertion order 
    std::vector<std::pair<K,mesh::armature>> armature_data;     

public:

    mesh_group() = default;
    mesh_group(mesh_group&&) = default;           //  movable..
    mesh_group& operator=(mesh_group&&) = delete; //  not assignable..
           
    template <typename VBuffer, typename IBuffer>
    void add_mesh(K key, VBuffer const& v, IBuffer const& i, unsigned weights_per_vertex) 
    {
        auto it = find_if(begin(offset_data),end(offset_data),[&](auto const& e) { return e.first == key; });
        if (it != end(offset_data)) throw std::runtime_error{"mesh_group::add_mesh() key already inserted..\n"}; 
        
        offset_data.emplace_back(std::move(key), entry{static_cast<unsigned>(vertex_buffer.size()), 
                                                       static_cast<unsigned>(index_buffer.size()), 
                                                       static_cast<unsigned>(v.size()), 
                                                       static_cast<unsigned>(i.size()), 
                                                       weights_per_vertex});        
        using std::begin; using std::end;
        vertex_buffer.insert(end(vertex_buffer), begin(v), end(v));
        index_buffer.insert(end(index_buffer), begin(i), end(i));
    }

    void add_armature(K key, mesh::armature armature_) 
    {
        auto it = find_if(begin(armature_data),end(armature_data),[&](auto const& e) { return e.first == key; });
        if (it != end(armature_data)) throw std::runtime_error{"mesh_group::add_armature() key already inserted..\n"};         
        armature_data.emplace_back(std::move(key), std::move(armature_));        
    }

    template <typename F>
    void apply(K const& key, F func) const
    {        
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& e) { return e.first == key; });
        if (it == end(offset_data)) throw std::runtime_error{"mesh_group::apply() key not found..\n"};         
        auto const& entry_ = *it;        
        func(entry_.second);
    }

    template <typename F>
    void apply(F func) const
    {
        for (auto&& e : offset_data) {        
            func(e.second);
        }
    }

    unsigned index_of_mesh(K const& key) const { 
        auto it = find_if(begin(offset_data), end(offset_data), [&](auto const& e) { return e.first == key; });
        if (it == end(offset_data)) throw std::runtime_error{"mesh_group key not found..\n"};        
        return static_cast<unsigned>(std::distance(begin(offset_data),it));
    }

    auto vertex_data() const noexcept
    {
        return reinterpret_cast<char const*>(vertex_buffer.data());
    }    
    
    auto vertex_data_size() const noexcept 
    {
        return vertex_buffer.size() * sizeof(V);
    }

    auto index_data_size() const noexcept 
    {
        return index_buffer.size() * sizeof(I);
    }

    auto index_data() const noexcept 
    {
        return reinterpret_cast<char const*>(index_buffer.data());        
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
