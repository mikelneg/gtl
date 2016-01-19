#ifndef YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_
#define YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d
    class synchronization_object;    
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <gtl/include/d3d_types.h>
#include <cassert>
#include <atomic>

namespace gtl {
namespace d3d {

class synchronization_object {
    gtl::d3d::command_queue& cqueue_;
    gtl::d3d::fence fence_;            
    uint64_t last_set_value_{};    
    uint64_t const allowed_latency_{};
    uint64_t const cycle_length_{};
            
public:
    
    synchronization_object(gtl::d3d::command_queue&, 
                           unsigned max_value_in_cycle,                     // e.g., if cycle is [0,1,2,3] max is 3, 
                           unsigned tolerated_difference_in_values);        // e.g., if tolerated diff is 2, the actual completed cycle index can lag behind
                                                                            //       the submitted index sequence by 2; beyond that the desync call is issued until 
                                                                            //       the completed cycle index is back within the tolerance         
    template <typename F, typename G>
    void operator()(F&& sync_call, G&& desync_call) {            
        // sync_call(value) should return true if the synchronization_object should advance its value, false otherwise.
        // desync_call() is a placeholder for the most part and will be called if the sync_object's value tolerance has been exceeded
        //  (this might be useful for logging..)
        auto period = [](auto const& value, auto const& length){ return value % length; };

        if (values_are_synchronized()) {    // synchronized case    
            if (sync_call(period(last_set_value_,cycle_length_))) {     // if sync_call(value) returns true we advance   
                cqueue_->Signal(fence_.get(), last_set_value_);        // value N has been issued to the fence..
                ++last_set_value_;                                    // prepare to issue N+1..
            } else {}                                                // otherwise we do nothing..                                                                                    
        } else {                // desychronized case
            desync_call();      // issue sympathy call
        }
    }    

    synchronization_object(synchronization_object&&) = delete;
    synchronization_object& operator==(synchronization_object&&) = delete;

    ~synchronization_object() {
        wait_for_values_to_sync_at(0);  // synchronizes destruction with the command_queue
    }

private:

    inline bool values_are_synchronized() const {
        // TODO this could possibly be improved by caching the fence's value and only polling when necessary..
        uint64_t diff = last_set_value_ - fence_->GetCompletedValue();
        return (diff <= allowed_latency_);
    }

    inline void wait_for_values_to_sync_at(uint64_t new_value) 
    {        
        last_set_value_ = new_value;
        fence_.synchronized_set(last_set_value_, cqueue_);                
        assert(last_set_value_ == fence_->GetCompletedValue());
    }

};


}} // namespaces
#endif

