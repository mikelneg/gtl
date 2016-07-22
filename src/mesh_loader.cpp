#include "gtl/mesh_loader.h"

#include <iostream>
#include <functional>
#include <cassert>
#include <memory>

#include <Eigen/Core>

#include <fbxsdk.h>

namespace gtl {
    
    namespace {
        template <typename T>
        using fbx_ptr = std::unique_ptr<T,std::function<void(T*)>>;       
    }


    struct mesh_loader::priv_impl {
        
        using bone = std::pair<uint32_t, float>; 

        std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>> vertices;                
        std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>> normals;                
        std::vector<unsigned> indices;
        std::vector<std::vector<bone>> bones; 

        priv_impl(std::string filename, gtl::tags::fbx_format) 
        {                   
            auto destroy_ptr = [](auto* p){ if (p) p->Destroy(); };
            auto do_nothing_ptr = [](auto*){};

            fbx_ptr<FbxManager> manager{FbxManager::Create(),destroy_ptr};
            fbx_ptr<FbxIOSettings> ios{FbxIOSettings::Create(manager.get(), IOSROOT),destroy_ptr};    
            manager->SetIOSettings(ios.get());    
            fbx_ptr<FbxImporter> importer{FbxImporter::Create(manager.get(),""),destroy_ptr};
            importer->Initialize(filename.c_str(),-1, manager->GetIOSettings());    
            fbx_ptr<FbxScene> scene{FbxScene::Create(manager.get(),"whatever"),destroy_ptr};
            importer->Import(scene.get());
            fbx_ptr<FbxNode> root_node{scene->GetRootNode(),do_nothing_ptr};            
            fbx_ptr<FbxNode> child{root_node->GetChild(0),do_nothing_ptr};

            auto print_vec = [](auto v, unsigned sz = 3){ 
                std::cout << "(";
                for (unsigned i = 0; i < sz; ++i) {
                    std::cout << v[i] << ":";
                }
                std::cout << ")";
            };
    
            auto m_ptr = child->GetNodeAttribute();
            auto m_type = m_ptr->GetAttributeType();
            if (m_type == FbxNodeAttribute::eMesh) {
                fbx_ptr<FbxMesh> mesh_loader{static_cast<FbxMesh*>(child->GetNodeAttribute()),do_nothing_ptr};                    
                    
                    mesh_loader->GenerateNormals(true,true,false);  // HACK needs better coordination between left/right handed transforms..

                    auto* points = mesh_loader->GetControlPoints();                 
                    
                     

                    //auto& norm = mesh_loader->GetElementNormal(0)->GetIndexArray();
                    
                    auto& norm = mesh_loader->GetLayer(0)->GetNormals()->GetDirectArray();


                    for (int i = 0; i < mesh_loader->GetControlPointsCount(); ++i) {
                        auto& p = points[i];
                        vertices.emplace_back(static_cast<float>(p[0]), static_cast<float>(p[1]), -1.0f * static_cast<float>(p[2]),1.0f);                                                                                                
                        //vertices.emplace_back(static_cast<float>(p[0]),static_cast<float>(p[2]), -1.0f * static_cast<float>(p[1]),1.0f);                                                                                                
                        auto n = norm.GetAt(i);
                        n.Normalize();
                        //normals.emplace_back(-1.0f * static_cast<float>(n[0]), -1.0f * static_cast<float>(n[1]), static_cast<float>(n[2]),0.0f);
                        normals.emplace_back(static_cast<float>(n[0]),static_cast<float>(n[1]), -1.0f * static_cast<float>(n[2]),0.0f);
                        //normals.emplace_back(static_cast<float>(n[0]), static_cast<float>(n.mData[2]), static_cast<float>(n.mData[1]),0.0f);
                    }                              
                    
                    
                    for (int i = 0; i < mesh_loader->GetPolygonCount(); ++i) {                                                                
                        //indices.emplace_back(mesh_loader->GetPolygonVertex(i,0));                        
                        //indices.emplace_back(mesh_loader->GetPolygonVertex(i,2));                        
                        //indices.emplace_back(mesh_loader->GetPolygonVertex(i,1));                        
                        for (int j = 0, sz = mesh_loader->GetPolygonSize(0); j < sz; ++j) {                            
                            indices.emplace_back(mesh_loader->GetPolygonVertex(i,j));                        
                        }                                                
                    }
              
                    fbx_ptr<FbxSkin> skin{static_cast<FbxSkin*>(mesh_loader->GetDeformer(0,FbxDeformer::eSkin)), do_nothing_ptr};                                              
                    for (int i = 0; i < skin->GetClusterCount(); ++i) {
                        auto cluster = skin->GetCluster(i);
                        auto ids = cluster->GetControlPointIndices();                        
                        auto wghts = cluster->GetControlPointWeights();

                        std::vector<bone> b;

                        for (int j = 0; j < cluster->GetControlPointIndicesCount(); ++j) {    
                            b.emplace_back(static_cast<uint32_t>(ids[j]),static_cast<float>(wghts[j]));                            
                        }
                        bones.emplace_back(std::move(b));                              
                    }        
            } else { 
                std::cout << "Not a mesh_loader..\n"; 
            }         
        }
    };

    mesh_loader::mesh_loader(std::string filename, gtl::tags::fbx_format) 
        : impl_{std::make_unique<priv_impl>(std::move(filename), gtl::tags::fbx_format{})}
    {}
    
    size_t mesh_loader::bone_count() const 
    { 
        return impl_->bones.size();
    }

    mesh_loader::aligned_vector<Eigen::Vector4f> 
    mesh_loader::vertices() const 
    {
        return impl_->vertices;
    }

    mesh_loader::aligned_vector<vertex_type_bone> 
    mesh_loader::bone_vertices() const 
    {
        std::vector<vertex_type_bone,Eigen::aligned_allocator<vertex_type_bone>> ret;        

        auto v_beg = begin(impl_->vertices);
        auto n_beg = begin(impl_->normals);

        for (unsigned i = 0, j = static_cast<unsigned>(impl_->vertices.size()); i < j; ++i, ++v_beg, ++n_beg) {
            ret.emplace_back(vertex_type_bone{*v_beg,*n_beg,Eigen::Vector4i{9999,9999,9999,9999},Eigen::Vector4f{0.0f,0.0f,0.0f,0.0f}});            
        }

        auto const& bones = impl_->bones;

        for (unsigned i = 0, j = static_cast<unsigned>(bones.size()); i < j; ++i) {  // vec<vec<pair<id,weight>>; 
            for (auto&& e : bones[i]) {                // vec<pair<id,weight>>
                ret[e.first].bone_ids[i] = e.first;    // 
                ret[e.first].bone_weights[i] = e.second;    // 
            }
        }
        
        return ret;
    }

    std::vector<unsigned> mesh_loader::indices() const 
    {
        return impl_->indices;
    }

    mesh_loader::~mesh_loader() {}

}

