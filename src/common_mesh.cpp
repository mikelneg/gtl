/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/common_mesh.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include <Eigen/Core>

#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/optional.hpp>

#include <gtl/fbx_loader.h>

namespace gtl {
namespace mesh {

    namespace {

        inline 
        Eigen::Matrix4f convert_matrix_handedness(fbxsdk::FbxAMatrix const& in) {                                                   
            //  { rx, ry, rz, 0 }  
            //  { ux, uy, uz, 0 }  
            //  { lx, ly, lz, 0 }  
            //  { px, py, pz, 1 }
            //     vvvvvvvvvv
            //  { rx, rz, ry, 0 }  
            //  { lx, lz, ly, 0 }  
            //  { ux, uz, uy, 0 }  
            //  { px, pz, py, 1 }             
            Eigen::Matrix4f out;                        
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

        inline 
        Eigen::Vector4f convert_vector_handedness(fbxsdk::FbxVector4 const& in, float w) {                   
            return Eigen::Vector4f{ static_cast<float>(in[0]), static_cast<float>(in[1]), static_cast<float>(in[2]), w};                                     
        }

        template <typename T>
        inline 
        void normalize_bones(T& bones) {
            for (auto& e : bones) {
                e.bone_weights_.normalize();
            }
        }
        
        template <typename T>
        inline 
        void flip_face_orientation(T& indices) {
            for (int i = 0, sz = static_cast<int>(indices.size()); i < sz-2; i+=3) { 
                std::swap(indices[i+1],indices[i+2]);        
            }
        }   
    }
    
    struct mesh_loader::priv_impl {        
        gtl::mesh::fbx::fbx_loader loader_;

        decltype(auto) loader() { return loader_; }

        priv_impl(std::string filename) 
            : loader_(std::move(filename)) {}        
    };

    mesh_loader::mesh_loader(std::string filename, tags::mesh_format_fbx)
        : impl_{std::make_unique<priv_impl>(std::move(filename))} 
    {}
    


    //### private implementation ########################
    /*
    struct mesh_loader::priv_impl {            
    
        struct control_point_bone {
            
            void push_bone(unsigned id, float weight)
            {
                if (id > 3) { return; } // TODO handle too many/too few bones..                
                bone_indexes_[current_bone] = id;
                bone_weights_[current_bone] = weight;    
                current_bone++;
            }
    
            control_point_bone() = default;    
            Eigen::Vector4i bone_indexes_{0,0,0,0};
            Eigen::Vector4f bone_weights_{0.0f,0.0f,0.0f,0.0f};            
            unsigned current_bone{0};
        };
            
        std::vector<Eigen::Vector4f> v_positions;
        std::vector<Eigen::Vector4f> v_normals;
        std::vector<Eigen::Vector2f> v_uvs;
        std::vector<bone> v_bones;        

        std::vector<control_point_bone> control_bones;    
        std::vector<Eigen::Vector2f> control_uvs;   

        std::vector<Eigen::Matrix4f> bone_transforms;
        
        std::vector<uint32_t> indices;        
           
        Eigen::Matrix4f mesh_transform;    
        unsigned number_of_bones;        

        bool has_uvs;
    
        void extract_mesh(FbxNode* child)                        
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
                        v_bones.emplace_back(control_bones[control_point_index].bone_indexes_, 
                                             control_bones[control_point_index].bone_weights_);                      
                    }
                }                                      

                flip_face_orientation(indices);            
        }


        priv_impl(std::string filename, gtl::tags::mesh_format_fbx)
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
                        
            if (m_type == FbxNodeAttribute::eMesh) extract_mesh(child);   

            //FbxMesh* m;         
            //m->GetDeformer(0,FbxDeformer::eSkin);
          
            //FbxSkin* s;
            //FbxCluster* c;            
            //auto v = m->GetElementVertexColor(0);

            //v->Get
            
            //s->GetMappingMode();
            //child->EvaluateGlobalBoundingBoxMinMaxCenter(vec4boxmin,vec4boxmax,vec4boxcent)
            // if it is a mesh we should spin off a mesh loader at this point.. 
            // if it is a armature we should spin off a armature loader.. etc
            
        }
    };

*/

//size_t mesh_loader::bone_count() const {
//    return static_cast<size_t>(impl_->bone_count());
//}
//
//std::vector<Eigen::Vector4f> mesh_loader::vertex_positions() const {
//    return impl_->vertex_positions();
//}
//
//std::vector<Eigen::Vector2f> mesh_loader::vertex_uvs() const {
//    return impl_->vertex_uvs();
//}

template <typename T, typename U, typename V>
static T replace_vertex_origins_with_bones(T const& v_positions, U& v_bones, V const& bone_transforms) {

    T new_positions;

    for (size_t i = 0; i < v_positions.size(); ++i) {
        
        Eigen::Vector4f tmp{0.0f,0.0f,0.0f,0.0f},con{0.0f,0.0f,0.0f,1.0f};                  

        tmp += (bone_transforms.at(v_bones[i].first[0]).transform_ * con) * v_bones[i].second[0];
        tmp += (bone_transforms.at(v_bones[i].first[1]).transform_ * con) * v_bones[i].second[1];
        tmp += (bone_transforms.at(v_bones[i].first[2]).transform_ * con) * v_bones[i].second[2];
        tmp += (bone_transforms.at(v_bones[i].first[3]).transform_ * con) * v_bones[i].second[3];             
                
        tmp = v_positions[i] - tmp;
        tmp.w() = 1.0f;
        
        new_positions.emplace_back(tmp);
    }

    return new_positions;
}

std::vector<renderer_vertex_type> 
mesh_loader::assembled_vertices() const {
    
    std::vector<renderer_vertex_type> ret;

    auto convert_vector = [](auto const& v) { return decltype(v){v[0], v[2], v[1], v[3]}; };
    auto convert_uv = [](auto const& v) { return decltype(v){v[0], 1.0f - v[1]}; };

    auto positions = impl_->loader_.vertex_positions();
    auto normals = impl_->loader_.vertex_normals();
    auto bones = impl_->loader_.bones();    
    auto uvs = impl_->loader_.uvs();

    auto armature = impl_->loader_.convert_armature();

    assert(positions.size() == normals.size() &&
           positions.size() == uvs.size() && 
           positions.size() == bones.size());
        
    // normalize vertex weights..
    for (auto&& e : bones) { e.second.normalize(); }        

    //std::transform(begin(positions),end(positions),begin(positions),convert_vector);
    //std::transform(begin(normals),end(normals),begin(normals),convert_vector);

    positions = replace_vertex_origins_with_bones(positions,bones,armature);    
    
    auto v_beg = begin(positions);
    auto n_beg = begin(normals);
    auto uv_beg = begin(uvs);    

    for (unsigned i = 0, sz = static_cast<unsigned>(positions.size()); i < sz; ++i, ++v_beg, ++n_beg, ++uv_beg)
    {
        ret.emplace_back(renderer_vertex_type{*v_beg, *n_beg, bones[i].first, bones[i].second, convert_uv(*uv_beg)});
    }
    return ret;
}

std::vector<uint32_t> mesh_loader::indices() const
{   
    auto indices = impl_->loader().indices();
    flip_face_orientation(indices);
    return indices;
}

boost::container::flat_map<bone::id_type,bone> 
mesh_loader::armature() const {
    return impl_->loader_.convert_armature();
}

mesh_loader::~mesh_loader() {}  // needed for PIMPL

}}
