#include "gtl/mesh.h"

#include <functional>
#include <memory>
#include <iostream>

#include <Eigen/Core>

#include <fbxsdk.h>


#include <cassert>

namespace gtl {
    
    namespace {
        template <typename T>
        using fbx_ptr = std::unique_ptr<T,std::function<void(T*)>>;
    }


    struct mesh::priv_impl {
        
        using bone = std::pair<uint32_t, float>; 

        std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>> vertices;                
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
                fbx_ptr<FbxMesh> mesh{static_cast<FbxMesh*>(child->GetNodeAttribute()),do_nothing_ptr};                    
                    auto* points = mesh->GetControlPoints();                                         
                    for (int i = 0; i < mesh->GetControlPointsCount(); ++i) {
                        auto& p = points[i];
                        vertices.emplace_back(static_cast<float>(p[0]),static_cast<float>(p[1]),static_cast<float>(p[2]),1.0f);                        
                    }          


                    for (int i = 0; i < mesh->GetPolygonCount(); ++i) {                                                                
                        for (int j = 0, sz = mesh->GetPolygonSize(0); j < sz; ++j) {                            
                            indices.emplace_back(mesh->GetPolygonVertex(i,j));                        
                        }                                                
                    }
              
                    fbx_ptr<FbxSkin> skin{static_cast<FbxSkin*>(mesh->GetDeformer(0,FbxDeformer::eSkin)), do_nothing_ptr};                                              
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
                std::cout << "Not a mesh..\n"; 
            }         
        }
    };

    mesh::mesh(std::string filename, gtl::tags::fbx_format) 
        : impl_{std::make_unique<priv_impl>(std::move(filename), gtl::tags::fbx_format{})}
    {}
     
    std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>> mesh::vertices() const 
    {
        return impl_->vertices;
    }

    std::vector<vertex_type_bone,Eigen::aligned_allocator<vertex_type_bone>> mesh::bone_vertices() const 
    {
        std::vector<vertex_type_bone,Eigen::aligned_allocator<vertex_type_bone>> ret;        

        auto v_beg = begin(impl_->vertices);

        for (unsigned i = 0, j = static_cast<unsigned>(impl_->vertices.size()); i < j; ++i, ++v_beg) {
            ret.emplace_back(vertex_type_bone{*v_beg,Eigen::Vector4i{9999,9999,9999,9999},Eigen::Vector4f{0.0f,0.0f,0.0f,0.0f}});            
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

    std::vector<unsigned> mesh::indices() const 
    {
        return impl_->indices;
    }




    mesh::~mesh() {}

}

