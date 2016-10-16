/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/mesh_loader.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <algorithm>

#include <Eigen/Core>

#include <fbxsdk.h>

namespace gtl {

    namespace {
        
    
        struct destroy_deleter {
            template <typename T>
            void operator()(T* t) { if (t) t->Destroy(); }
        };
    
        template <typename T>
        using fbx_ptr = std::unique_ptr<T, destroy_deleter>;
                
        inline Eigen::Matrix4f convert_matrix(fbxsdk::FbxAMatrix const& in) {   
                static_assert(sizeof(decltype(in)) == sizeof(double)*16, "fbxsdk::FbxAMatrix (a 4x4 matrix of doubles) is not contiguous..");                
            Eigen::Matrix4f out;                        
            //auto* in_data = in.Buffer()->Buffer();                  
            //std::transform(in_data, in_data+16, out.data(), [](auto const& v){ return static_cast<float>(v); }); 

            // right-to-left handed conversion
            out(0,0) = static_cast<float>(in.Get(0,0)); 
            out(0,1) = static_cast<float>(in.Get(0,2)); 
            out(0,2) = static_cast<float>(in.Get(0,1)); 
            out(0,3) = static_cast<float>(in.Get(0,3)); 
            out(1,0) = static_cast<float>(in.Get(2,0)); 
            out(1,1) = static_cast<float>(in.Get(2,2)); 
            out(1,2) = static_cast<float>(in.Get(2,1)); 
            out(1,3) = static_cast<float>(in.Get(2,3)); 
            out(2,0) = static_cast<float>(in.Get(1,0)); 
            out(2,1) = static_cast<float>(in.Get(1,2)); 
            out(2,2) = static_cast<float>(in.Get(1,1)); 
            out(2,3) = static_cast<float>(in.Get(1,3)); 
            out(3,0) = static_cast<float>(in.Get(3,0)); 
            out(3,1) = static_cast<float>(in.Get(3,2)); 
            out(3,2) = static_cast<float>(in.Get(3,1)); 
            out(3,3) = static_cast<float>(in.Get(3,3));             

            return out;
        }

        inline Eigen::Vector4f convert_vector(fbxsdk::FbxVector4 const& in, float w) {                   
            return Eigen::Vector4f{ static_cast<float>(in[0]), static_cast<float>(in[2]), -static_cast<float>(in[1]), w};                                     
        }

        template <typename T>
        inline void normalize_bones(T& bones) {
            for (auto& e : bones) {
                e.bone_weights_.normalize();
            }
        }
        
        template <typename T>
        inline void flip_face_orientation(T& indices) {
            for (int i = 0, sz = static_cast<int>(indices.size()); i < sz-2; i+=3) { 
                std::swap(indices[i+1],indices[i+2]);
                //auto tmp = o[i+1]; 
                //o[i+1] = o[i+2]; 
                //o[i+2] = tmp;   
            }
        }
    }

    //### private implementation ########################
    struct mesh_loader::priv_impl {            
    
        struct control_point_bone {
            
            void push_bone(unsigned id, float weight)
            {
                if (id > 3) { return; } // TODO handle too many/too few bones..                
                //bone_indexes_[current_bone] = id;
                bone_weights_[id] = weight;    
                //current_bone++;
            }
    
            control_point_bone() = default;    
            //Eigen::Vector4i bone_indexes_{0,0,0,0};
            Eigen::Vector4f bone_weights_{0.0f,0.0f,0.0f,0.0f};            
            //unsigned current_bone{0};
        };
            
        aligned_vector<Eigen::Vector4f> v_positions;
        aligned_vector<Eigen::Vector4f> v_normals;
        aligned_vector<Eigen::Vector2f> v_uvs;
        aligned_vector<bone> v_bones;        


        aligned_vector<control_point_bone> control_bones;    
        aligned_vector<Eigen::Vector2f> control_uvs;   

        aligned_vector<Eigen::Matrix4f> bone_transforms;
        
        std::vector<uint32_t> indices;        
           
        Eigen::Matrix4f mesh_transform;    
        unsigned number_of_bones;

        bool has_uvs;
    
        //

        priv_impl(std::string filename, gtl::tags::fbx_format)
        {            
            fbx_ptr<FbxManager> manager{FbxManager::Create()};
            fbx_ptr<FbxIOSettings> ios{FbxIOSettings::Create(manager.get(), IOSROOT)};
                manager->SetIOSettings(ios.get());
            fbx_ptr<FbxImporter> importer{FbxImporter::Create(manager.get(), "")};
                importer->Initialize(filename.c_str(), -1, manager->GetIOSettings());
            fbx_ptr<FbxScene> scene{FbxScene::Create(manager.get(), "whatever")};
                importer->Import(scene.get());
            
            FbxNode* root_node{scene->GetRootNode()};
            FbxNode* child{root_node->GetChild(0)};
    
            auto m_type = child->GetNodeAttribute()->GetAttributeType();            
                        
            if (m_type == FbxNodeAttribute::eMesh)
            {                                
                mesh_transform = convert_matrix(child->EvaluateGlobalTransform());                

                auto mesh_loader = static_cast<fbxsdk::FbxMesh*>(child->GetNodeAttribute());                                
                auto skin = static_cast<FbxSkin*>(mesh_loader->GetDeformer(0, FbxDeformer::eSkin));
                                                                 
                mesh_loader->GenerateNormals(true, true, true); // Generated v_normals are clockwise                        
    
                control_bones.resize(mesh_loader->GetControlPointsCount());                 
                number_of_bones = skin->GetClusterCount();

                for (int bone_idx = 0; bone_idx < skin->GetClusterCount(); ++bone_idx)
                {
                    auto* cluster = skin->GetCluster(bone_idx);
    
                    fbxsdk::FbxAMatrix trans_link_tmp,trans_tmp;
                    cluster->GetTransformLinkMatrix(trans_link_tmp);          
                    cluster->GetTransformMatrix(trans_tmp);
                    bone_transforms.emplace_back(convert_matrix(trans_tmp * trans_link_tmp));                    
    
                    auto* index_ = cluster->GetControlPointIndices();
                    auto* weight_ = cluster->GetControlPointWeights();                                                                                                  

                    for (int j = 0; j < cluster->GetControlPointIndicesCount(); ++j) {
                        control_bones[index_[j]].push_bone(bone_idx, static_cast<float>(weight_[j]));
                    }                    
                }    
                              
                // load uvs..
                has_uvs = mesh_loader->GetUVLayerCount() > 0;                 

                if (has_uvs) {                                        
                    fbxsdk::FbxStringList names;
                    mesh_loader->GetUVSetNames(names);                                                                                                                                 

                    auto* uv = static_cast<fbxsdk::FbxLayerElementUV*>(mesh_loader->GetElementUV("UVMap"));                         
                    //auto uv_count = mesh_loader->GetElementUVCount();
                    
                    //FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvs_{};                                        
                    //mesh_loader->GetTextureUV(&uvs_);          
        
                    //std::cout << uv->GetMappingMode() << " mapping mode.. \n";
                    //std::cout << uv->GetReferenceMode() << " reference mode.. \n"; 

                    switch (uv->GetMappingMode()) {                   
                        case fbxsdk::FbxLayerElement::EMappingMode::eByControlPoint : break;                        
                        case fbxsdk::FbxLayerElement::EMappingMode::eByPolygonVertex : 
                        {
                                switch(uv->GetReferenceMode()) {
                                    case fbxsdk::FbxLayerElement::EReferenceMode::eIndexToDirect : 
                                        {                                          
                                            for (int i = 0; i < mesh_loader->GetPolygonCount(); ++i) {
                                                for (int j = 0, sz = mesh_loader->GetPolygonSize(i); j < sz; ++j) {                
                                                    // ... GetTextureUVIndex requires eByPolygonVertex mode..                                            
                                                    bool unmapped{};
                                                    //FbxVector2 tmp = uvs_->GetAt(mesh_loader->GetTextureUVIndex(i,j));    
                                                    FbxVector2 tmp{};
                                                    mesh_loader->GetPolygonVertexUV(i,j,names[0].Buffer(),tmp,unmapped);
                                                    v_uvs.emplace_back(tmp[0],1.0f - tmp[1]);                                                          
                                                }
                                            }
                                        } break;
                                    default: break;
                                }                            
                        } break;
                        default : break;
                    }                                                                           
                } else {
                    for (int i = 0; i < mesh_loader->GetPolygonCount(); ++i) {
                        for (int j = 0, sz = mesh_loader->GetPolygonSize(i); j < sz; ++j) {                
                            v_uvs.emplace_back(0.0f,0.0f);                                                          
                        }
                    }                    
                }
                                
                for (int i = 0; i < mesh_loader->GetPolygonCount(); ++i) {
                    for (int j = 0, sz = mesh_loader->GetPolygonSize(i); j < sz; ++j) {
                
                        auto control_point_index = mesh_loader->GetPolygonVertex(i, j);                                           
                        v_positions.emplace_back(convert_vector(mesh_loader->GetControlPointAt(control_point_index),1.0f)); // w == 1.0f
                                                                                               
                        fbxsdk::FbxVector4 tmp_normal{0.0,0.0,0.0,0.0}; 

                        if (mesh_loader->GetPolygonVertexNormal(i, j, tmp_normal)) {                        
                            v_normals.emplace_back(convert_vector(tmp_normal,0.0f)); // w == 0.0f
                        } else {
                            v_normals.emplace_back(0.0f,0.0f,0.0f,0.0f); 
                        }                        

                        indices.emplace_back(j + (i * sz)); // control_point_index );   // ignoring actual index for now..                                                                                                   
                        v_bones.emplace_back(//control_bones[control_point_index].bone_indexes_, 
                                             control_bones[control_point_index].bone_weights_);                      
                    }
                }                                      

                flip_face_orientation(indices);
            }
            else
            {
                std::cout << "Not a mesh..\n";
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

mesh_loader::aligned_vector<Eigen::Vector4f> mesh_loader::vertex_positions() const
{
    return impl_->v_positions;
}

mesh_loader::aligned_vector<Eigen::Vector2f> mesh_loader::vertex_uvs() const
{
    return impl_->v_uvs;
}


template <typename T, typename U, typename V>
static T replace_vertex_origins_with_bones(T const& v_positions, U& v_bones, V const& bone_transforms) {

    T new_positions;

    for (size_t i = 0; i < v_positions.size(); ++i) {
        
        Eigen::Vector4f tmp{0.0f,0.0f,0.0f,0.0f},con{0.0f,0.0f,0.0f,1.0f};                

        tmp += (bone_transforms[0] * con) * v_bones[i][0];
        tmp += (bone_transforms[1] * con) * v_bones[i][1];
        tmp += (bone_transforms[2] * con) * v_bones[i][2];
        tmp += (bone_transforms[3] * con) * v_bones[i][3];             
                
        tmp = tmp - v_positions[i];
        tmp.w() = 1.0f;
        
        new_positions.emplace_back(tmp);
    }

    return new_positions;
}

mesh_loader::aligned_vector<vertex_type_bone> mesh_loader::bone_vertices() const
{
    std::vector<vertex_type_bone, Eigen::aligned_allocator<vertex_type_bone>> ret;

    assert(impl_->v_positions.size() == impl_->v_normals.size() &&
           impl_->v_positions.size() == impl_->v_uvs.size() && 
           impl_->v_positions.size() == impl_->v_bones.size());
    
    for (auto&& e : impl_->v_bones) { if (e.norm() < 1.0f) { e.x() = 1.0f; } e.normalize(); }
    impl_->v_positions = replace_vertex_origins_with_bones(impl_->v_positions,impl_->v_bones, impl_->bone_transforms);

    auto v_beg = begin(impl_->v_positions);
    auto n_beg = begin(impl_->v_normals);
    auto uv_beg = begin(impl_->v_uvs);    

    for (unsigned i = 0, j = static_cast<unsigned>(impl_->v_positions.size()); i < j; ++i, ++v_beg, ++n_beg, ++uv_beg)
    {
        ret.emplace_back(vertex_type_bone{*v_beg, *n_beg, impl_->v_bones[i], *uv_beg});
    }
    return ret;
}

mesh_loader::aligned_vector<mesh_loader::bone> mesh_loader::bones() const {
    return impl_->v_bones;
}

std::vector<uint32_t> mesh_loader::indices() const
{    
    return impl_->indices;
}

mesh_loader::aligned_vector<Eigen::Matrix4f> mesh_loader::links() const
{
    return impl_->bone_transforms;
}

Eigen::Matrix4f mesh_loader::mesh_transform() const
{
    return impl_->mesh_transform;
}

mesh_loader::~mesh_loader()
{
}
}
