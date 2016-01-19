#include "../include/synchronization_object.h"

#include <cstdint>
#include <cassert>
#include <atomic>
#include <gtl/include/d3d_types.h>
#include <gtl/include/d3d_funcs.h>


/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace d3d {

synchronization_object::synchronization_object(command_queue& cqueue_, unsigned max_value_in_cycle, unsigned allowed_desync_)
:   cqueue_{cqueue_},
    fence_{gtl::d3d::get_device_from(cqueue_)},                            
    allowed_latency_{allowed_desync_},  
    cycle_length_{max_value_in_cycle + 1}   
{               
    assert(allowed_latency_ <= cycle_length_);       
    wait_for_values_to_sync_at(0);                
}    


}} // namespaces