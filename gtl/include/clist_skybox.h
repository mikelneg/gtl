#ifndef HWUIWFOAVVFSF_GTL_CLIST_SKYBOX_H_
#define HWUIWFOAVVFSF_GTL_CLIST_SKYBOX_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0 
    
    useful types
-----------------------------------------------------------------------------*/

#include <gtl/include/d3d_types.h>

namespace gtl {
namespace d3d {
namespace command_lists {

    void skybox_command_list(d3d::command_list& clist, 
                         d3d::direct_command_allocator& alloc, 
                         d3d::pipeline_state_object& pso, 
                         d3d::root_signature& rsig, 
                         D3D12_VIEWPORT const& viewport,
                         D3D12_RECT const& scissor_rect,
                         d3d::rtv_frame_resources& rtv,
                         d3d::swap_chain& swchain,
                         d3d::rtv_descriptor_heap& rtvheap);

}}} // namespaces
#endif
