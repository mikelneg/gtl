/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UTUIOWAQABVV_GTL_COMMON_MESH_H_
#define UTUIOWAQABVV_GTL_COMMON_MESH_H_

#include <memory>
#include <string>
#include <vector>
#include <tuple>

#include <ostream>

#include <Eigen/Core>

#include <boost/container/static_vector.hpp>

#include <gtl/tags.h>

namespace gtl {

struct renderer_vertex_type { // HACK find a better way to organize this..
    Eigen::Vector4f pos;
    Eigen::Vector4f normal;
    Eigen::Vector4i bone_ids;
    Eigen::Vector4f bone_weights;
    Eigen::Vector2f uv;
};

namespace tags {
    struct mesh_format_fbx {
    };
}

namespace mesh {

    using vertex_id = uint32_t;
    using vertex_bone_data = std::pair<Eigen::Vector4i, Eigen::Vector4f>;

    struct bone {
        static constexpr int max_number_of_children = 5;
        static constexpr int root_id()
        {
            return -1;
        }

        using id_type = int;

        Eigen::Matrix4f transform_;
        Eigen::Vector4f tail_;
        id_type parent_id_;
        boost::container::static_vector<id_type, max_number_of_children> children_ids_;

        bone(Eigen::Matrix4f const& t, Eigen::Vector4f const& tail_position, id_type parent_id) : transform_(t), tail_(tail_position), parent_id_(parent_id)
        {
        }

        void add_child(id_type child_id)
        {
            children_ids_.emplace_back(child_id);
        }
        friend std::ostream& operator<<(std::ostream& str, bone const& b)
        {
            return str << b.parent_id_ << ",";
        }
    };

    using armature = boost::container::flat_map<int, bone>;

    class mesh_loader {
        struct priv_impl;
        std::unique_ptr<priv_impl> impl_;

    public:
        using bone_type = std::pair<Eigen::Vector4i, Eigen::Vector4f>;

        mesh_loader(std::string filename, tags::mesh_format_fbx);
        ~mesh_loader();

        boost::container::flat_map<std::string, mesh::bone::id_type> armature_ids() const;
        boost::container::flat_map<bone::id_type, bone> armature() const;
        std::vector<renderer_vertex_type> assembled_vertices() const;
        std::vector<uint32_t> indices() const;
        static constexpr int bone_count()
        {
            return 4;
        }
    };
}
} // namespaces
#endif
