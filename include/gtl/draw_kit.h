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

//#include <algorithm>
#include <string>

#include <Eigen/Geometry>

//#include <gtl/events.h>
//#include <gtl/win_keyboard.h>

//#include <vn/boost_variant_utilities.h>
#include <vn/swap_object.h>

namespace gtl {

class draw_data {
public:
    struct vertex_type {
        Eigen::Vector4f position;
        Eigen::Vector4f color;
        vertex_type() = default;
        vertex_type(Eigen::Vector4f const& p, Eigen::Vector4f const& c = {1.0f, 1.0f, 0.2f, 0.2f}) noexcept : position(p), color(c)
        {
        }
    };

    using index_type = uint16_t;

private:
    std::vector<vertex_type> vertices_;
    std::vector<index_type> indices_;

    std::size_t vertex_count_{};
    std::size_t index_count_{};
    index_type index_offset_{}; // only valid if used between clear() and
                                // add_index_group
public:
    decltype(auto) vertices()
    {
        return vertices_;
    }
    decltype(auto) indices()
    {
        return indices_;
    }

    void add_vertex(vertex_type v)
    {
        vertices_.emplace_back(v);
        vertex_count_++;
    }

    void add_index_group(std::initializer_list<index_type> il_)
    {
        for (auto&& i : il_)
        {
            indices_.emplace_back(i + index_offset_);
        }
        index_count_++;
        index_offset_ += static_cast<index_type>(il_.size());
    }

    void add_triangle(index_type a, index_type b, index_type c)
    {
        indices_.emplace_back(a + index_offset_);
        indices_.emplace_back(b + index_offset_);
        indices_.emplace_back(c + index_offset_);
        index_count_ += 3;
        // index_offset_ += il_.size();
    }

    void next_group()
    {
        index_offset_ = static_cast<index_type>(vertices_.size());
    }

    void draw_line(Eigen::Vector3f a, Eigen::Vector3f b, Eigen::Vector3f color, float line_width = 0.03f, float alpha = 0.3f)
    {
        //    const float line_width = 0.06f;
        auto ba = b - a;
        float len = ba.norm();

        const Eigen::Vector3f pts[]{{0.0f, line_width, 0.0f}, {len, line_width, 0.0f}, {len, -line_width, 0.0f}, {0.0f, -line_width, 0.0f},
                                    {0.0f, 0.0f, line_width}, {len, 0.0f, line_width}, {len, 0.0f, -line_width}, {0.0f, 0.0f, -line_width}};

        const index_type indices[]{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};

        auto rotation = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f{1.0f, 0.0f, 0.0f}, ba);

        for (auto&& e : pts)
        {
            auto e_rot = rotation._transformVector(e);
            e_rot += a;
            // add_vertex(vertex_type{Eigen::Vector4f{e_rot.x(),e_rot.y(),e_rot.z(),0.0f}, color});
            vertices_.emplace_back(Eigen::Vector4f{e_rot.x(), e_rot.y(), e_rot.z(), 1.0f}, Eigen::Vector4f{color.x(), color.y(), color.z(), alpha});
            vertex_count_++;
        }
        add_triangle(0, 1, 2);
        add_triangle(0, 2, 3);
        add_triangle(4, 5, 6);
        add_triangle(4, 6, 7);
        next_group();
        // add_index_group({0,1,2,0,2,3,4,5,6,4,6,7});
        //   next_group();
    }

    void draw_triangle(Eigen::Vector4f p0, Eigen::Vector4f p1, Eigen::Vector4f p2)
    {
        add_vertex({p0, {1.0f, 1.0f, 1.0f, 0.6f}});
        add_vertex({p1, {1.0f, 1.0f, 1.0f, 0.6f}});
        add_vertex({p2, {1.0f, 1.0f, 1.0f, 0.6f}});
        add_triangle(0, 1, 2);
        next_group();
    }

    void draw_plane(Eigen::Vector3f plane_normal, Eigen::Vector3f plane_origin)
    {
        // add_vertex({{-1.0f,0.0f,-1.0f, 0.0f}});
        // add_vertex({{-1.0f,0.0f,1.0f,0.0f}});
        // add_vertex({{ 0.0f,0.0f,0.0f,1.0f}});
        // add_vertex({{ 1.0f,0.0f,-1.0f,0.0f}});
        // add_vertex({{-1.0f,0.0f,0.0f,0.0f}});
        // add_vertex({{0.0f,-1.0f,0.0f,0.0f}});
        // add_triangle(0,1,2);
        // add_triangle(0,2,3);
        ////add_triangle(0,3,4);
        // add_triangle(0,4,1);
        add_vertex({{-1000.f, 0.0f, -1000.0f, 1.0f}, {0.7f, 0.7f, 0.8f, 0.7f}});
        add_vertex({{-1000.0f, 0.0f, 1000.0f, 1.0f}, {0.2f, 0.2f, 0.2f, 0.5f}});
        add_vertex({{1000.0f, 0.0f, 1000.0f, 1.0f}, {0.2f, 0.2f, 0.2f, 0.5f}});
        add_vertex({{1000.0f, 0.0f, -1000.0f, 1.0f}, {0.7f, 0.7f, 0.8f, 0.7f}});
        add_triangle(0, 1, 2);
        add_triangle(0, 2, 3);

        next_group();
        // add_index_group({0,1,2,0,2,3,0,3,4,0,4,1});
    }

    void draw_axes()
    {
        draw_line({0.0f, 0.0f, 0.0f}, {10.0f, 0.0f, 0.0f}, {0.9f, 0.3f, 0.3f}, 0.2f, 0.4f);
        draw_line({0.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {0.3f, 0.9f, 0.3f}, 0.2f, 0.4f);
        draw_line({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 10.0f}, {0.3f, 0.3f, 0.9f}, 0.2f, 0.4f);
    }

    auto vertex_count() const noexcept
    {
        return (std::min)(vertex_count_, vertices_.size());
    }
    auto index_count() const noexcept
    {
        return (std::min)(index_count_, indices_.size());
    }
    void swap(draw_data& other)
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

class draw_kit {
    // TODO implement copy-free buffer updating (instead of swap_object<vector> or
    // whatever)
    vn::swap_object<draw_data> mutable output_buffer_;

public:
    draw_kit() = default;

    bool swap_out(draw_data& external)
    {
        return output_buffer_.swap_out(external);
    }

    void render(draw_data& ext_data) const
    {
        output_buffer_.swap_in(ext_data);
    }
};

} // namespaces
#endif
