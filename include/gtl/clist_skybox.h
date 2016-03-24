#ifndef HWUIWFOAVVFSF_GTL_CLIST_SKYBOX_H_
#define HWUIWFOAVVFSF_GTL_CLIST_SKYBOX_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0 
    
    useful types
-----------------------------------------------------------------------------*/

#include <gtl/d3d_types.h>

namespace gtl {
namespace d3d {
namespace graphics_command_lists {

    void skybox_graphics_command_list(d3d::graphics_command_list& clist, 
                         d3d::direct_command_allocator& alloc, 
                         d3d::pipeline_state_object& pso, 
                         d3d::root_signature& rsig, 
                         std::initializer_list<D3D12_VIEWPORT*> viewport,                         
                         D3D12_RECT const& scissor_rect,                             
                         d3d::resource& target_resource,                         
                         rtv_srv_texture2D& rtv_srv_,                         
                         d3d::resource& cbuf,            
                         d3d::resource_descriptor_heap& resource_heap,
                         d3d::sampler_descriptor_heap& sampler_heap);
    
    void skybox_graphics_command_list_second(d3d::graphics_command_list& clist,                                 
                         d3d::resource& target_resource,                         
                         d3d::uav_texture2D& rtv_srv_);
    

    class skybox {
            
    public:

    };

}}} // namespaces
#endif
