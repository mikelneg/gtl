#ifndef IRWFGHOSFF_GTL_D3D_FUNCS_H_
#define IRWFGHOSFF_GTL_D3D_FUNCS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0
    d3d support functions
-----------------------------------------------------------------------------*/

#include "d3d_default_implementation.h"
#include <gtl/include/release_ptr.h>

namespace gtl {    
namespace d3d {    

namespace _12_0 {           
            
        release_ptr<DXGIFactory> get_dxgi_factory();
        release_ptr<DXGIAdapter> get_hw_adapter();        
        
        DXGI_SWAP_CHAIN_DESC swchain_desc(HWND hwnd, size_t width, size_t height);    
 
        D3D12_COMMAND_QUEUE_DESC command_queue_desc();
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC swchain_fullscreen_desc();            

        D3D12_DESCRIPTOR_HEAP_DESC rtv_descriptor_heap_desc();
        D3D12_DESCRIPTOR_HEAP_DESC cbv_descriptor_heap_desc();

        void report_live_objects(class device&);
}

//using namespace gtl::d3d::default; 
}} // namespaces
#endif
