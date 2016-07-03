#ifndef UTUIOWAQABVV_GTL_MESH_H_
#define UTUIOWAQABVV_GTL_MESH_H_

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


class mesh {    
    struct priv_impl;
    std::unique_ptr<priv_impl> impl_;
public:       
    mesh(std::string filename, tags::fbx_format);
    ~mesh();

    std::vector<vertex_type_bone,Eigen::aligned_allocator<vertex_type_bone>> bone_vertices() const;
    std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f>> vertices() const;
    std::vector<unsigned> indices() const;
};

} // namespaces
#endif
