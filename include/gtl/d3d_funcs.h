#ifndef IRWFGHOSFF_GTL_D3D_FUNCS_H_
#define IRWFGHOSFF_GTL_D3D_FUNCS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0
    d3d support functions
-----------------------------------------------------------------------------*/

#include <gtl/d3d_include.h>
#include <gtl/d3d_types.h>
#include <gtl/tags.h>
#include <gtl/release_ptr.h>
#include <gtl/win_tools.h>

#include <ostream>
#include <vector>

namespace gtl {    
namespace d3d {    

namespace version_12_0 {           

        release_ptr<raw::Factory> get_dxgi_factory();        
        std::vector<raw::AdapterDesc> enumerate_adaptors();
        
        raw::SwapChainDesc create_swapchain_desc(tags::flipmodel_windowed, HWND, unsigned num_buffers, unsigned width, unsigned height);
 
        raw::CommandQueueDesc command_queue_desc();
        raw::SwapChainFullscreenDesc swchain_fullscreen_desc();            

        raw::DescriptorHeapDesc rtv_descriptor_heap_desc();
        raw::DescriptorHeapDesc resource_descriptor_heap_desc();

        void report_live_objects(class device&);
        void wait_for_gpu(device&,command_queue&);

        template <typename T>
        device get_device_from(T& t) { 
            device dev {gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type),reinterpret_cast<void**>(&dev.expose_ptr()))
                               ,__func__);                        
            return dev;
        }

        template <typename T>
        device get_device(T& t) { 
            device dev {gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type),reinterpret_cast<void**>(&dev.expose_ptr()))
                               ,__func__);                        
            return dev;
        }
        
        release_ptr<raw::Blob> dummy_rootsig_1();
        release_ptr<raw::Blob> dummy_rootsig_2();
        release_ptr<raw::Blob> dummy_rootsig_3();        
}

}} // namespaces
#endif
