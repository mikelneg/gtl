#ifndef IRWFGHOSFF_GTL_d3d_helper_funcs_H_
#define IRWFGHOSFF_GTL_d3d_helper_funcs_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0
    d3d support functions
-----------------------------------------------------------------------------*/

#include <gtl/d3d_version.h>
#include <gtl/d3d_types.h>
#include <gtl/tags.h>
#include <gtl/intrusive_ptr.h>
#include <gtl/win_tools.h>

#include <ostream>
#include <vector>

namespace gtl {    
namespace d3d {    
namespace version_12_0 {           
       
        // Nice discussion, and suggests 3 as the minimum default buffer count (reasons in the video):    
        // https://www.youtube.com/watch?v=E3wTajGZOsA
        inline int frame_count() noexcept { return 3; }

        std::vector<raw::AdapterDesc> enumerate_adaptors();
        
        //raw::SwapChainDesc create_swapchain_desc(tags::flipmodel_windowed, HWND, unsigned num_buffers, unsigned width, unsigned height);
 
        //raw::CommandQueueDesc command_queue_desc();
        //raw::SwapChainFullscreenDesc swchain_fullscreen_desc();            
        //
        //raw::DescriptorHeapDesc rtv_descriptor_heap_desc();
        //raw::DescriptorHeapDesc resource_descriptor_heap_desc();

        void report_live_objects(device&);
        
        void wait_for_gpu(device&,command_queue&);
        void wait_for_gpu(command_queue&);
        
        template <typename T>
        device get_device_from(T& t) { 
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type),expose_as_void_pp(dev))
                               ,__func__);                        
            return dev;
        }

        template <typename T>
        device get_device_from(T* t) { 
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type),expose_as_void_pp(dev))
                               ,__func__);                        
            return dev;
        }
        
        template <typename T>
        device get_device(T& t) { 
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type),expose_as_void_pp(dev))
                               ,__func__);                        
            return dev;
        }                        
}

}} // namespaces
#endif
