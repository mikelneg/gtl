#ifndef UTUIOWAQABVV_GTL_MESH_LOADER_H_
#define UTUIOWAQABVV_GTL_MESH_LOADER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    mesh type
-----------------------------------------------------------------------------*/

#include <vector>
#include <string>
#include <memory>
#include <Eigen/Core>
#include <gtl/tags.h>

namespace gtl {    
    namespace tags {
        struct fbx_format {};
    }

    struct vertex_type_bone {
        Eigen::Vector4f pos;
        Eigen::Vector4i bone_ids;
        Eigen::Vector4f bone_weights;
    };

    class mesh_loader {               // HACK hackish.. 
        template <typename T>
        using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;        
        
        struct priv_impl;
        std::unique_ptr<priv_impl> impl_;

    public:       
        mesh_loader(std::string filename, tags::fbx_format);
        ~mesh_loader();
    
        aligned_vector<vertex_type_bone> bone_vertices() const;
        aligned_vector<Eigen::Vector4f> vertices() const;
        std::vector<unsigned> indices() const;
        size_t bone_count() const;
    };

} // namespaces
#endif
