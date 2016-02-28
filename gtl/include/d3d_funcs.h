#ifndef IRWFGHOSFF_GTL_D3D_FUNCS_H_
#define IRWFGHOSFF_GTL_D3D_FUNCS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0
    d3d support functions
-----------------------------------------------------------------------------*/

#include "d3d_default_implementation.h"
#include <gtl/include/d3d_types.h>
#include <gtl/include/release_ptr.h>
#include <gtl/include/win_tools.h>

namespace gtl {    
namespace d3d {    

namespace _12_0 {           

        namespace tags {
            struct flipmodel_windowed{};
        }

        release_ptr<DXGIFactory> get_dxgi_factory();
        release_ptr<DXGIAdapter> get_hw_adapter();                
        
        DXGI_SWAP_CHAIN_DESC create_swapchain_desc(tags::flipmodel_windowed, HWND, unsigned num_buffers, unsigned width, unsigned height);
 
        D3D12_COMMAND_QUEUE_DESC command_queue_desc();
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC swchain_fullscreen_desc();            

        D3D12_DESCRIPTOR_HEAP_DESC rtv_descriptor_heap_desc();
        D3D12_DESCRIPTOR_HEAP_DESC resource_descriptor_heap_desc();

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
        
        release_ptr<D3DBlob> dummy_rootsig_1();
        release_ptr<D3DBlob> dummy_rootsig_2();
        release_ptr<D3DBlob> dummy_rootsig_3();
        
}

//using namespace gtl::d3d::default; 
}} // namespaces
#endif
