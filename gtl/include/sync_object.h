#ifndef YUWOQWFAVF_GTL_D3D_SYNCH_OBJECT_H_
#define YUWOQWFAVF_GTL_D3D_SYNCH_OBJECT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d

    class sync_object;
    
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <windows.h>
#include <gtl/include/d3d_default_implementation.h>
#include <gtl/include/d3d_types.h>
#include <gtl/include/release_ptr.h>
#include <gtl/include/win_tools.h>
#include <type_traits>
#include <exception>
#include <cassert>
#include <atomic>

namespace gtl {
namespace d3d {

class sync_object {
    command_queue& cqueue_;
    fence fence_;            
    uint64_t last_set_value_{};    
    uint64_t const allowed_latency_{};
    uint64_t const cycle_length_{};
        
    void wait_for_sync_values(uint64_t new_value) {        
        last_set_value_ = new_value;
        fence_.synchronized_set(last_set_value_,cqueue_);                
        assert(last_set_value_ == fence_->GetCompletedValue());
    }

public:

    sync_object(command_queue& cqueue_, fence fence_, unsigned int max_value_in_cycle, unsigned int allowed_desync_) 
        :   cqueue_{cqueue_},
            fence_{std::move(fence_)},                            
            allowed_latency_{allowed_desync_},  // specifies the allowed gap between last_set_value_ and the fence's value; anything more causes values_in_sync() to return false  
            cycle_length_{max_value_in_cycle + 1}   
    {               
        assert(allowed_latency_ <= cycle_length_);       
        wait_for_sync_values(0);                
    }    

    bool values_in_sync() {            
        // overflow with uint64_t is not feasible, so we ignore it
        uint64_t diff = last_set_value_ - fence_->GetCompletedValue();
        return (diff <= allowed_latency_);        
    }    

    template <typename F, typename G>
    void operator()(F&& sync_call, G&& desync_call) {            
        auto period = [](auto const& value, auto const& length){ return value % length; };

        if (values_in_sync()) {                                         // completed value must be within allowed_latency_ to synchronize
            if (sync_call(period(last_set_value_,cycle_length_))) {      // returns true if we should advance values..                
                cqueue_->Signal(fence_.get(), last_set_value_);          //     value N issued..
                ++last_set_value_;                                       //     get ready for value N+1..
            } else {                                                     //
                // do nothing                                            // returns false otherwise..
            }        
        } else { 
            desync_call();                                               // if we were desynced in the beginning, call desync_call() and then do nothing..
        }
    }    


    ~sync_object() {
        wait_for_sync_values(0);    // synchronizes destruction with command_queue
    }

};


}} // namespaces
#endif

