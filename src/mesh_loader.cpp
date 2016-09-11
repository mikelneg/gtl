/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/mesh_loader.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>

#include <Eigen/Core>

#include <fbxsdk.h>

namespace gtl {

namespace {
    template <typename T>
    using fbx_ptr = std::unique_ptr<T, std::function<void(T*)>>;
}

struct mesh_loader::priv_impl {

    using bone = std::pair<Eigen::Vector4i, Eigen::Vector4f>;

    struct control_point_bones {
        void push_bone(unsigned id, float weight)
        {
            if (current_bone > 3)
            {
                return;
            } // TODO handle too many/too few bones..
            deforming_bone_ids[current_bone] = id;
            deforming_bone_weights[current_bone] = weight;

            current_bone++;
        }

        control_point_bones() = default;

        Eigen::Vector4i deforming_bone_ids{};
        Eigen::Vector4f deforming_bone_weights{};
        unsigned current_bone{};
    };

    std::vector<control_point_bones> cp_bones;

    std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> vertices;
    std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> normals;
    std::vector<uint32_t> indices;
    std::vector<bone> vertex_bones;

    std::vector<Eigen::Matrix4f> transform_links;
    Eigen::Matrix4f mesh_transform;

    unsigned number_of_bones;

    priv_impl(std::string filename, gtl::tags::fbx_format)
    {
        auto destroy_ptr = [](auto* p) {
            if (p)
                p->Destroy();
        };
        auto do_nothing_ptr = [](auto*) {};

        fbx_ptr<FbxManager> manager{FbxManager::Create(), destroy_ptr};
        fbx_ptr<FbxIOSettings> ios{FbxIOSettings::Create(manager.get(), IOSROOT), destroy_ptr};
        manager->SetIOSettings(ios.get());
        fbx_ptr<FbxImporter> importer{FbxImporter::Create(manager.get(), ""), destroy_ptr};
        importer->Initialize(filename.c_str(), -1, manager->GetIOSettings());
        fbx_ptr<FbxScene> scene{FbxScene::Create(manager.get(), "whatever"), destroy_ptr};
        importer->Import(scene.get());
        fbx_ptr<FbxNode> root_node{scene->GetRootNode(), do_nothing_ptr};
        fbx_ptr<FbxNode> child{root_node->GetChild(0), do_nothing_ptr};

        auto m_ptr = child->GetNodeAttribute();
        auto m_type = m_ptr->GetAttributeType();
        if (m_type == FbxNodeAttribute::eMesh)
        {

            fbx_ptr<FbxMesh> mesh_loader{static_cast<FbxMesh*>(child->GetNodeAttribute()), do_nothing_ptr};
            fbx_ptr<FbxSkin> skin{static_cast<FbxSkin*>(mesh_loader->GetDeformer(0, FbxDeformer::eSkin)),
                                  do_nothing_ptr};

            mesh_loader->GenerateNormals(true, true, false);

            cp_bones.resize(mesh_loader->GetControlPointsCount());

            number_of_bones = skin->GetClusterCount();

            for (unsigned bone_idx = 0, sz = skin->GetClusterCount(); bone_idx < sz; ++bone_idx)
            {
                auto* cluster = skin->GetCluster(bone_idx);

                auto* deforming_indices = cluster->GetControlPointIndices();
                auto* deforming_weights = cluster->GetControlPointWeights();

                //
                fbxsdk::FbxAMatrix trans_link_tmp, mesh_trans;
                cluster->GetTransformLinkMatrix(trans_link_tmp);
                cluster->GetTransformMatrix(mesh_trans);

                auto trans_link = trans_link_tmp;

                Eigen::Matrix4f matrix;
                for (int w = 0; w < 4; ++w)
                {
                    for (int y = 0; y < 4; ++y)
                    {
                        matrix(w, y) = static_cast<float>(trans_link.Get(w, y));
                        mesh_transform(w, y) = static_cast<float>(mesh_trans.Get(w, y));
                    }
                }

                transform_links.emplace_back(matrix);

                //

                for (unsigned j = 0, sx = cluster->GetControlPointIndicesCount(); j < sx; ++j)
                {
                    cp_bones[deforming_indices[j]].push_bone(bone_idx, static_cast<float>(deforming_weights[j]));
                }
            }

            for (int i = 0; i < mesh_loader->GetPolygonCount(); ++i)
            {
                for (int j = 0, sz = mesh_loader->GetPolygonSize(i); j < sz; ++j)
                {

                    auto control_point_index = mesh_loader->GetPolygonVertex(i, j);
                    auto position = mesh_loader->GetControlPointAt(control_point_index);

                    fbxsdk::FbxVector4 normal;
                    mesh_loader->GetPolygonVertexNormal(i, j, normal);
                    normal.Normalize();

                    vertices.emplace_back(static_cast<float>(position[0]), static_cast<float>(position[1]),
                                          -1.0f * static_cast<float>(position[2]), 1.0f);

                    normals.emplace_back(static_cast<float>(normal[0]), static_cast<float>(normal[1]),
                                         -1.0f * static_cast<float>(normal[2]), 0.0f);

                    indices.emplace_back(j + (i * sz)); // TODO optimize meshes

                    auto& bone_data = cp_bones[control_point_index];

                    vertex_bones.emplace_back(bone_data.deforming_bone_ids, bone_data.deforming_bone_weights);
                }
            }
        }
        else
        {
            std::cout << "Not a mesh_loader..\n";
        }
    }
};

mesh_loader::mesh_loader(std::string filename, gtl::tags::fbx_format)
    : impl_{std::make_unique<priv_impl>(std::move(filename), gtl::tags::fbx_format{})}
{
}

size_t mesh_loader::bone_count() const
{
    return impl_->number_of_bones;
}

mesh_loader::aligned_vector<Eigen::Vector4f> mesh_loader::vertices() const
{
    return impl_->vertices;
}

mesh_loader::aligned_vector<vertex_type_bone> mesh_loader::bone_vertices() const
{
    std::vector<vertex_type_bone, Eigen::aligned_allocator<vertex_type_bone>> ret;

    auto v_beg = begin(impl_->vertices);
    auto n_beg = begin(impl_->normals);

    for (unsigned i = 0, j = static_cast<unsigned>(impl_->vertices.size()); i < j; ++i, ++v_beg, ++n_beg)
    {
        ret.emplace_back(vertex_type_bone{*v_beg, *n_beg, impl_->vertex_bones[i].first, impl_->vertex_bones[i].second});
    }
    return ret;
}

std::vector<uint32_t> mesh_loader::indices() const
{
    return impl_->indices;
}

std::vector<Eigen::Matrix4f> mesh_loader::links() const
{
    return impl_->transform_links;
}

Eigen::Matrix4f mesh_loader::mesh_transform() const
{
    return impl_->mesh_transform;
}

mesh_loader::~mesh_loader()
{
}
}
