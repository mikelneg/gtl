#include "gtl/synchronization_object.h"

#include <cstdint>
#include <cassert>
#include <atomic>
#include <gtl/d3d_types.h>
#include <gtl/d3d_funcs.h>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace d3d {
namespace _12_0 {


synchronization_object::synchronization_object(command_queue& cqueue_, unsigned max_value_in_cycle, unsigned allowed_desync_)
:   cqueue_{cqueue_},
    fence_{gtl::d3d::get_device_from(cqueue_)},                            
    allowed_latency_{allowed_desync_},  
    cycle_length_{max_value_in_cycle + 1}   
{               
    assert(allowed_latency_ <= cycle_length_ - 2);       
    wait_for_values_to_sync_at(0);                
}    


synchronization_object::~synchronization_object()
{
    wait_for_values_to_sync_at(0);
}


unsigned synchronization_object::periodic_value() 
const 
{     
    return static_cast<unsigned>(last_set_value_ % cycle_length_); 
}


void synchronization_object::advance() 
{ 
    cqueue_->Signal(fence_.get(), last_set_value_); ++last_set_value_; 
}

bool synchronization_object::values_are_synchronized() 
const 
{
    // TODO it's possible this could be improved by caching the fence's value and only polling when necessary..
    uint64_t const diff = last_set_value_ - fence_->GetCompletedValue();
    return (diff <= allowed_latency_);
}

void synchronization_object::wait_for_values_to_sync_at(uint64_t new_value) 
{        
    last_set_value_ = new_value;
    fence_.synchronized_set(last_set_value_, cqueue_);                
    assert(last_set_value_ == fence_->GetCompletedValue());
}

}}} // namespaces