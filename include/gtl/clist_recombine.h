#ifndef HWUIWFOAVVFSF_GTL_CLIST_RECOMBINE_H_
#define HWUIWFOAVVFSF_GTL_CLIST_RECOMBINE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0       
-----------------------------------------------------------------------------*/

#include <gtl/include/d3d_types.h>

namespace gtl {
namespace d3d {
    namespace graphics_command_lists {

        void recombine_graphics_command_list(d3d::graphics_command_list& clist,
                                             d3d::direct_command_allocator& alloc,
                                             d3d::pipeline_state_object& pso,
                                             d3d::cb_root_signature& rsig,
                                             std::initializer_list<D3D12_VIEWPORT*> viewport,
                                             D3D12_RECT const& scissor_rect,
                                             d3d::resource& target_resource,
                                             rtv_srv_texture2D& rtv_srv_,
                                             d3d::resource& cbuf,
                                             d3d::resource_descriptor_heap& cbvheap,
                                             d3d::sampler_descriptor_heap& smpheap);
    }
}
} // namespaces
#endif
