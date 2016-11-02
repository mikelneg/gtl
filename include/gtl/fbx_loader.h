/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BPQOWFAQWFWEF_GTL_FBX_LOADER_H_
#define BPQOWFAQWFWEF_GTL_FBX_LOADER_H_

#include <string>
#include <vector>
#include <memory>

#include <Eigen/Geometry>

#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/optional.hpp>

#include <gtl/common_mesh.h>

#include <fbxsdk.h>

namespace gtl {
namespace mesh {
    namespace fbx {

        struct fbx_bone {
            static constexpr int max_number_of_children = gtl::mesh::bone::max_number_of_children;
            static std::string root_id()
            {
                return "root";
            }
            using id_type = std::string;

            FbxMatrix g_transform_;
            FbxMatrix l_transform_;
            float length_;

            boost::container::static_vector<std::string, max_number_of_children> children_;
            id_type parent_;

            fbx_bone(FbxMatrix const& global_transform, FbxMatrix const& local_transform, float length, id_type parent)
                : g_transform_{global_transform}, l_transform_{local_transform}, length_{length}, parent_{std::move(parent)}
            {
            }

            void add_child(std::string child)
            {
                children_.emplace_back(std::move(child));
            }

            std::pair<FbxVector4, FbxVector4> head_and_tail() const
            {
                return std::make_pair(g_transform_.MultNormalize({0.0, 0.0, 0.0, 1.0}), g_transform_.MultNormalize({length_, 0.0, 0.0, 1.0}));
            }

            friend std::ostream& operator<<(std::ostream& str, fbx_bone const& b)
            {
                return str << b.parent_ << ",";
            }
        };

        struct fbx_vertex_bone_data {
            Eigen::Vector4i indices_{0, 0, 0, 0};
            Eigen::Vector4f weights_{0, 0, 0, 0};
            unsigned current_bone_{0};
            fbx_vertex_bone_data() = default;

            void push_bone(unsigned id, float weight)
            {
                if (weight < 0.0001f)
                    return;
                if (current_bone_ > 3)
                { // TODO check for too many/too few bones..
                    return;
                }
                indices_[current_bone_] = id;
                weights_[current_bone_] = weight;
                current_bone_++;
            }
        };

        class fbx_loader {

            boost::container::flat_map<std::string, fbx_bone> armature_;

            boost::optional<boost::container::flat_map<std::string, mesh::bone::id_type>> bone_names_;
            boost::optional<boost::container::flat_map<mesh::bone::id_type, fbx_vertex_bone_data>> vertex_bone_data_;
            boost::optional<boost::container::flat_map<vertex_id, Eigen::Vector4f>> vertex_color_data_;

            std::vector<Eigen::Vector4f> positions_;
            std::vector<Eigen::Vector4f> normals_;
            std::vector<mesh::vertex_id> indices_;
            std::vector<mesh::vertex_bone_data> bones_;

            boost::optional<std::vector<Eigen::Vector4f>> colors_;
            boost::optional<std::vector<Eigen::Vector2f>> uvs_;

            void load_mesh(FbxMesh*);
            void load_armature(FbxNode*, fbxsdk::FbxAMatrix const&);

        public:
            fbx_loader(std::string filename);

            auto vertex_positions() const
            {
                return positions_;
            }
            auto vertex_normals() const
            {
                return normals_;
            }
            auto indices() const
            {
                return indices_;
            }
            auto bones() const
            {
                return bones_;
            }

            gtl::mesh::armature convert_armature() const;

            auto armature_ids() const
            {
                if (!bone_names_)
                {
                    throw std::runtime_error{"fbx_loader::armature_ids() requires bone_names_"};
                }
                else
                    return *bone_names_;
            }

            std::vector<Eigen::Vector2f> uvs() const
            {
                if (uvs_)
                    return *uvs_;
                else
                    return {};
            }
            std::vector<Eigen::Vector4f> colors() const
            {
                if (colors_)
                    return *colors_;
                else
                    return {};
            }
        };
    }
}
} // namespaces
#endif
