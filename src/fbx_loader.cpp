/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/fbx_loader.h"

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

#include <Eigen/Geometry>

#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/optional.hpp>

#include <fbxsdk.h>

namespace gtl {
namespace mesh {
namespace fbx {
   
    namespace {
        inline 
        Eigen::Matrix4f convert_matrix(fbxsdk::FbxMatrix const& in) {                                                   
            Eigen::Matrix4f out;               
            std::transform(static_cast<double const*>(in),
                           static_cast<double const*>(in)+16,
                           out.data(),
                           [](auto const& v) { return static_cast<float>(v); });
            return out;
        }
        Eigen::Vector4f convert_vector(FbxVector4 const& v, float w) { return {static_cast<float>(v[0]),static_cast<float>(v[1]),static_cast<float>(v[2]), w}; }
        Eigen::Vector4f convert_color(FbxColor const& c) { return {static_cast<float>(c[0]),static_cast<float>(c[1]),static_cast<float>(c[2]), static_cast<float>(c[3])}; }                
        
        inline 
        Eigen::Matrix4f convert_matrix_handedness(fbxsdk::FbxMatrix const& in) {                                                   
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
    
        struct destroy_deleter {
        template <typename T>
            void operator()(T* t) { if (t) t->Destroy(); }
        };
    
        template <typename T>
        using fbx_ptr = std::unique_ptr<T, destroy_deleter>;               

        std::string name(FbxNodeAttribute::EType e) {
            switch(e) {
            case FbxNodeAttribute::EType::eUnknown          : return "eUnknown";
            case FbxNodeAttribute::EType::eNull             : return "eNull";
            case FbxNodeAttribute::EType::eMarker           : return "eMarker"; 
            case FbxNodeAttribute::EType::eSkeleton         : return "eSkeleton"; 
            case FbxNodeAttribute::EType::eMesh             : return "eMesh"; 
            case FbxNodeAttribute::EType::eNurbs            : return "eNurbs";
            case FbxNodeAttribute::EType::ePatch            : return "ePatch";
            case FbxNodeAttribute::EType::eCamera           : return "eCamera";
            case FbxNodeAttribute::EType::eCameraStereo     : return "eCameraStereo";
            case FbxNodeAttribute::EType::eCameraSwitcher   : return "eCameraSwitcher";
            case FbxNodeAttribute::EType::eLight            : return "eLight";
            case FbxNodeAttribute::EType::eOpticalReference : return "eOpticalReference";
            case FbxNodeAttribute::EType::eOpticalMarker    : return "eOpticalMarker";
            case FbxNodeAttribute::EType::eNurbsCurve       : return "eNurbsCurve";
            case FbxNodeAttribute::EType::eTrimNurbsSurface : return "eTrimNurbsSurface";
            case FbxNodeAttribute::EType::eBoundary         : return "eBoundary";
            case FbxNodeAttribute::EType::eNurbsSurface     : return "eNurbsSurface";
            case FbxNodeAttribute::EType::eShape            : return "eShape";
            case FbxNodeAttribute::EType::eLODGroup         : return "eLODGroup";
            case FbxNodeAttribute::EType::eSubDiv           : return "eSubDiv";
            case FbxNodeAttribute::EType::eCachedEffect     : return "eCachedEffect";
            case FbxNodeAttribute::EType::eLine             : return "eLine";
            default : return "Unknown_FbxNodeAttribute";
            }
        }               

    } // namespace

    
    void fbx_loader::load_mesh(FbxMesh* node) 
    {         
        auto get_vertex_bone_data = [](FbxMesh& mesh) -> boost::optional<boost::container::flat_map<mesh::bone::id_type,fbx_vertex_bone_data>> {
            if (mesh.GetDeformerCount() == 0) return {};             

            boost::container::flat_map<mesh::bone::id_type,fbx_vertex_bone_data> vertex_bone_data_;
            auto &skin = *static_cast<FbxSkin*>(mesh.GetDeformer(0,FbxDeformer::eSkin));

            for (int bone_idx = 0; bone_idx < skin.GetClusterCount(); ++bone_idx) {
                auto* cluster = skin.GetCluster(bone_idx);
                auto* indices = cluster->GetControlPointIndices();
                auto* weights = cluster->GetControlPointWeights();                            
                for (int v_idx = 0; v_idx < cluster->GetControlPointIndicesCount(); ++v_idx) {
                    vertex_bone_data_[indices[v_idx]].push_bone(bone_idx, static_cast<float>(weights[v_idx]));
                }
            }
            return vertex_bone_data_; // one element per control-point from the mesh; each specifies up to four bone indices and their weights
        };

        auto get_bone_names = [](FbxMesh& mesh) -> boost::optional<boost::container::flat_map<std::string,mesh::bone::id_type>> {
            if (mesh.GetDeformerCount() == 0) return {};            
            
            boost::container::flat_map<std::string,mesh::bone::id_type> bone_names_;
            auto &skin = *static_cast<FbxSkin*>(mesh.GetDeformer(0,FbxDeformer::eSkin));
            
            for (int bone_idx = 0; bone_idx < skin.GetClusterCount(); ++bone_idx) {
                auto* cluster = skin.GetCluster(bone_idx);
                //bone_names_[bone_idx] = cluster->GetLink()->GetName();                                                                  
                bone_names_[cluster->GetLink()->GetName()] = bone_idx;                                                                  
            }            
            return bone_names_; // one element per bone; maps the bone's index to its name
        };        

        auto get_uvs = [](FbxMesh& mesh) -> boost::optional<std::vector<Eigen::Vector2f>> {                
            if (mesh.GetElementUVCount() == 0) return {};

            FbxStringList names;
            mesh.GetUVSetNames(names);                               
            std::vector<Eigen::Vector2f> uvs_;
            for (int i = 0; i < mesh.GetPolygonCount(); ++i) {
                for (int j = 0, sz = mesh.GetPolygonSize(i); j < sz; ++j) {                                                                                                        
                    bool unmapped{}; 
                    FbxVector2 tmp{0.0,0.0};
                    mesh.GetPolygonVertexUV(i,j,names[0].Buffer(),tmp,unmapped);
                    uvs_.emplace_back(static_cast<float>(tmp[0]),static_cast<float>(tmp[1]));                                                          
                }
            }                       
            return uvs_;                                                    
        };

        auto get_colors = [](FbxMesh& mesh) -> boost::optional<boost::container::flat_map<vertex_id,Eigen::Vector4f>> {                
            if (mesh.GetElementVertexColorCount() == 0) return {};
            
            boost::container::flat_map<vertex_id,Eigen::Vector4f> colors_;
            
            auto& color_layer = *static_cast<FbxGeometryElementVertexColor*>(mesh.GetElementVertexColor(0));            
            auto& direct_arr = color_layer.GetDirectArray();
            auto& index_arr = color_layer.GetIndexArray();
            auto map_type = color_layer.GetReferenceMode();

            switch (map_type) {
            case FbxLayerElement::EReferenceMode::eIndexToDirect : {
                    for (int i = 0; i < index_arr.GetCount(); ++i) {
                        colors_[ index_arr[i] ] = convert_color(direct_arr[ index_arr[i] ]);
                    }                    
                } break;
            case FbxLayerElement::EReferenceMode::eDirect : {
                    for (int i = 0; i < direct_arr.GetCount(); ++i) {
                        colors_[i] = convert_color(direct_arr[i]);
                    }                    
                } break;
             default: break;
            }            
            return colors_;                                                     
        };


        auto get_vertex_data = [&](FbxMesh* mesh) {
            for (int i = 0; i < mesh->GetPolygonCount(); ++i) {
                for (int j = 0, sz = mesh->GetPolygonSize(i); j < sz; ++j) {                    
                    auto control_point_index = mesh->GetPolygonVertex(i,j);                                                                       
                
                    fbxsdk::FbxVector4 tmp_normal{0.0,0.0,0.0,0.0}; 
                    mesh->GetPolygonVertexNormal(i,j,tmp_normal);

                    positions_.emplace_back(convert_vector(mesh->GetControlPointAt(control_point_index),1.0f)); // w == 1.0f                        
                    normals_.emplace_back(convert_vector(tmp_normal,0.0f)); // w == 0.0f; also, if normals are not mapped tmp_normal remains unchanged                                                                                
                    indices_.emplace_back(j + (i * sz)); // control_point_index );   // ignoring actual index for now..                                                                                                   
                    
                    if (vertex_bone_data_) {
                        bones_.emplace_back((*vertex_bone_data_)[control_point_index].indices_,
                                            (*vertex_bone_data_)[control_point_index].weights_);
                    }                      
                }
            }                                       
        };

        node->GenerateNormals(true,true,true); // Generates normals, vertex order is clockwise
                                                       
        vertex_bone_data_ = get_vertex_bone_data(*node);
        bone_names_ = get_bone_names(*node);                
        uvs_ = get_uvs(*node);
        vertex_color_data_ = get_colors(*node);
                              
        get_vertex_data(node);
    }

    void fbx_loader::load_skeleton(FbxNode* p) {

        std::vector<FbxNode*> children_;
        
        auto add_entry = [&](auto* node, auto const& parent_id) {
            skeleton_.try_emplace(node->GetName(),node->EvaluateGlobalTransform(),node->EvaluateLocalTransform(),node->GetSkeleton()->LimbLength.Get(),parent_id);                          
        };

        auto add_children = [&](auto* parent) {                
            for (int i = 0; i < parent->GetChildCount(); ++i) {
                auto* child = parent->GetChild(i);
                children_.emplace_back(child);                            
                add_entry(child,parent->GetName());                                
            }
        };        
                        
        add_entry(p,fbx_bone::root_id());

        add_children(p);

        while (!children_.empty()) {
            auto* back = children_.back(); 
            children_.pop_back();                        
            add_children(back);
        }

        auto build_children = [](auto& m) {
            for (auto&& e : m) {
                if (e.second.parent_ != fbx_bone::root_id())
                    m.at(e.second.parent_).add_child(e.first);            
            }
        };

        build_children(skeleton_);          
    }

    fbx_loader::fbx_loader(std::string filename) {                    

        fbx_ptr<FbxManager> manager{FbxManager::Create()};
        fbx_ptr<FbxIOSettings> ios{FbxIOSettings::Create(manager.get(), IOSROOT)};
        manager->SetIOSettings(ios.get());

        fbx_ptr<FbxImporter> importer{FbxImporter::Create(manager.get(), "")};
        importer->Initialize(filename.c_str(), -1, manager->GetIOSettings());

        fbx_ptr<FbxScene> scene{FbxScene::Create(manager.get(), "whatever")};
        importer->Import(scene.get());

        FbxNode* root_node{scene->GetRootNode()};

        std::vector<FbxNode*> children_; 
        auto add_children = [&](auto* p) {                
            for (int i = 0; i < p->GetChildCount(); ++i) {
                children_.emplace_back( p->GetChild(i) );
            }
        };   

        add_children(root_node);
                    
        while (!children_.empty()) {                
            auto back = children_.back(); children_.pop_back();
            auto m_type = back->GetNodeAttribute()->GetAttributeType();                                                 
            //std::cout << std::string(children_.size(),'-') << "type == " << name(m_type) << "\n";                            

            switch(m_type) {
            case FbxNodeAttribute::EType::eSkeleton : 
                {
                    load_skeleton(back);
                }   break;                
            case FbxNodeAttribute::EType::eMesh :
                {
                    load_mesh(static_cast<FbxMesh*>(back->GetNodeAttribute()));
                }   break;
            default : 
                {                
                    add_children(back);                    
                }
            }
        }            
    }                

    gtl::mesh::skeleton 
    fbx_loader::convert_skeleton() const {        

        if (!bone_names_) throw std::runtime_error{"fbx_loader::skeleton() requires bone_names_"};
        auto const& names_ = *bone_names_;        

        auto convert_bone = 
            [&](fbx_bone const& b) -> gtl::mesh::bone {               
                auto parent_id_ = names_.count(b.parent_) > 0 ? names_.at(b.parent_) : gtl::mesh::bone::root_id(); 
                gtl::mesh::bone return_bone{convert_matrix_handedness(b.g_transform_),
                                            convert_vector(b.head_and_tail().second,1.0f), 
                                            parent_id_};
                for (auto&& c : b.children_) {                     
                    return_bone.add_child(names_.at(c)); 
                }
                return return_bone;
            };

        gtl::mesh::skeleton ret;
        
        for (auto&& e : skeleton_) {
            if (names_.count(e.first) > 0)
                ret.try_emplace(names_.at(e.first),convert_bone(e.second)); 
        }

        return ret;
    }


}}} // namespaces