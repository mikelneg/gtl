#ifndef UTUIOWAQABVV_GTL_MESH_LOADER_H_
#define UTUIOWAQABVV_GTL_MESH_LOADER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    mesh type
-----------------------------------------------------------------------------*/

#include <memory>
#include <string>
#include <vector>

#include <ostream>

#include <Eigen/Core>
#include <gtl/tags.h>

namespace gtl {
namespace tags {
    struct fbx_format {
    };
}

struct vertex_type_bone {
    Eigen::Vector4f pos;
    Eigen::Vector4f normal; // HACK find a better way to organize this layout + direct3d input layout + etc..
    Eigen::Vector4i bone_ids;
    Eigen::Vector4f bone_weights;

    friend std::ostream& operator<<(std::ostream& str, vertex_type_bone const& b)
    {
        str << "pos(\n"
            << b.pos << ")\n";
        str << "norm(\n"
            << b.normal << ")\n";
        return str;
    }
};

class mesh_loader { // HACK hackish..
    template <typename T>
    using aligned_vector = std::vector<T, Eigen::aligned_allocator<T> >;

    struct priv_impl;
    std::unique_ptr<priv_impl> impl_;

public:
    mesh_loader(std::string filename, tags::fbx_format);
    ~mesh_loader();

    aligned_vector<vertex_type_bone> bone_vertices() const;
    aligned_vector<Eigen::Vector4f> vertices() const;
    std::vector<uint32_t> indices() const;
    std::vector<Eigen::Matrix4f> links() const;
    Eigen::Matrix4f mesh_transform() const;
    size_t bone_count() const;
};

} // namespaces
#endif
