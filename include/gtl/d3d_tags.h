#ifndef BOWOAFAZBBNASFAF_GTL_D3D_TAGS_H_
#define BOWOAFAZBBNASFAF_GTL_D3D_TAGS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::tags    
    various tag types used in gtl::d3d implementation

    TODO improve tag names..
-----------------------------------------------------------------------------*/

namespace gtl {
namespace d3d {
namespace tags {

    struct shader_visible{};
    struct not_shader_visible{};    
    struct flipmodel_windowed{};
    struct shader_view{};
    struct depth_stencil_view{};
    struct cbv_srv_uav{};

}}} // namespaces
#endif
