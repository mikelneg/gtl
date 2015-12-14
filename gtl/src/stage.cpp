#include "../include/stage.h"

#include <gtl/include/gtl_window.h>
#include <gtl/include/d3d_types.h>
#include <gtl/include/d3d_funcs.h>

#include <cassert>


/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/
                                  

namespace gtl {


stage::stage(gtl::d3d::swap_chain& swchain, unsigned num_buffers, unsigned max_desync)
    :   swchain_{swchain},
        cqueue_{get_device_from(swchain)},
        sync_{cqueue_,gtl::d3d::fence{get_device_from(swchain)},num_buffers-1,max_desync},
        num_buffers_{num_buffers}
{
    assert(num_buffers > 1);    
    


}


void stage::render()
{
//    std::initializer_list<ID3D12CommandList*> clists{cs_list_[idx].get()};                                         
//    if (WaitForSingleObject(swchain_->GetFrameLatencyWaitableObject(),0) == WAIT_OBJECT_0) {                                                         	                    
//        cqueue_->ExecuteCommandLists(gtl::win::array_size(clists), clists.begin());
//        swchain_->Present(0,0); 
//    }        
}





} // namespace