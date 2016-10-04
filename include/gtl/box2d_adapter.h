/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BLWKJALSFAEF_GTL_BOX2D_ADAPTER_H_
#define BLWKJALSFAEF_GTL_BOX2D_ADAPTER_H_

#include <cassert>
#include <chrono>
#include <utility>
#include <vector>

#include <algorithm>
#include <string>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <imgui.h>

#include <gtl/events.h>
#include <gtl/win_keyboard.h>
#include <vn/boost_variant_utilities.h>
#include <vn/swap_object.h>

#include <Box2D/Box2D.h>

namespace gtl {

class box2d_adapter {

public:
    struct imgui_data {
        std::vector<ImDrawVert> vertices_;
        std::vector<ImDrawIdx> indices_;

        std::size_t vertex_count_{};
        std::size_t index_count_{};
        ImDrawIdx index_offset_{}; // only valid if used between clear() and
                                   // add_index_group

        void add_vertex(ImDrawVert v)
        {
            vertices_.emplace_back(v);
            vertex_count_++;
        }

        void add_index_group(std::initializer_list<ImDrawIdx> il_)
        {
            for (auto&& i : il_)
            {
                indices_.emplace_back(i + index_offset_);
            }
            index_count_++;
            index_offset_ += static_cast<ImDrawIdx>(il_.size());
        }

        void add_triangle(ImDrawIdx a, ImDrawIdx b, ImDrawIdx c)
        {
            indices_.emplace_back(a + index_offset_);
            indices_.emplace_back(b + index_offset_);
            indices_.emplace_back(c + index_offset_);
            index_count_ += 3;
            // index_offset_ += il_.size();
        }

        void next_group()
        {
            index_offset_ = static_cast<ImDrawIdx>(vertices_.size());
        }

        auto vertex_count() const noexcept
        {
            return (std::min)(vertex_count_, vertices_.size());
        }
        auto index_count() const noexcept
        {
            return (std::min)(index_count_, indices_.size());
        }
        void swap(imgui_data& other)
        {
            using std::swap;
            swap(vertices_, other.vertices_);
            swap(indices_, other.indices_);
        }

        void clear()
        {
            vertices_.clear();
            vertex_count_ = 0;
            indices_.clear();
            index_count_ = 0;
            index_offset_ = 0;
        }
    };

private:
    // TODO implement copy-free buffer updating (instead of swap_object<vector> or
    // whatever)
    vn::swap_object<imgui_data> mutable output_buffer_;

    // imgui_data mutable local_data_;

public:
    box2d_adapter()
    {
    }

    // void dump_data(ImDrawData& draw_data)
    //{
    //    using std::begin;
    //    using std::end;
    //
    //    auto& verts = local_data_.vertices_;
    //    auto& indices = local_data_.indices_;
    //
    //    if (static_cast<int>(verts.size()) < draw_data.TotalVtxCount)
    //        verts.resize(draw_data.TotalVtxCount);
    //    if (static_cast<int>(indices.size()) < draw_data.TotalIdxCount)
    //        indices.resize(draw_data.TotalIdxCount);
    //
    //    local_data_.vertex_count_ = 0;
    //    local_data_.index_count_ = 0;
    //
    //    for (int n = 0, voffs = 0, ioffs = 0; n < draw_data.CmdListsCount; n++)
    //    {
    //        auto* cmd_list = draw_data.CmdLists[n];
    //
    //        assert(voffs < static_cast<int>(verts.size()));
    //        assert(ioffs < static_cast<int>(indices.size()));
    //
    //        std::copy(begin(cmd_list->VtxBuffer), end(cmd_list->VtxBuffer),
    //        begin(verts) + voffs);
    //        std::transform(begin(cmd_list->IdxBuffer), end(cmd_list->IdxBuffer),
    //        begin(indices) + ioffs, [&](auto const& v) { return v + voffs; });
    //
    //        voffs += cmd_list->VtxBuffer.size();
    //        ioffs += cmd_list->IdxBuffer.size();
    //
    //        local_data_.vertex_count_ += cmd_list->VtxBuffer.size();
    //        local_data_.index_count_ += cmd_list->IdxBuffer.size();
    //    }
    //
    //    output_buffer_.swap_in(local_data_);
    //}

    bool swap_out(imgui_data& external)
    {
        return output_buffer_.swap_out(external);
    }

    void render(imgui_data& ext_data) const
    {
        // collect vertex information

        // local_data_.vertices_.emplace_back(ImDrawVert{{0.0f,0.0f},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
        // local_data_.vertices_.emplace_back(ImDrawVert{{100.0f,0.0f},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
        // local_data_.vertices_.emplace_back(ImDrawVert{{0.0f,100.0f},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
        // local_data_.vertices_.emplace_back(ImDrawVert{{100.0f,100.0f},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
        //
        // local_data_.indices_.emplace_back(0);
        // local_data_.indices_.emplace_back(1);
        // local_data_.indices_.emplace_back(2);
        // local_data_.indices_.emplace_back(2);
        // local_data_.indices_.emplace_back(1);
        // local_data_.indices_.emplace_back(3);
        //
        // local_data_.vertex_count_ = 4;
        // local_data_.index_count_ = 6;

        output_buffer_.swap_in(ext_data);
    }
};

} // namespaces
#endif
