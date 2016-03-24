#ifndef YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_
#define YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d
    class synchronization_object;    
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <gtl/d3d_types.h>
#include <cassert>
#include <atomic>

namespace gtl {
namespace d3d {
namespace _12_0 {
    
class synchronization_object {    
    
    gtl::d3d::command_queue& cqueue_;
    gtl::d3d::fence fence_;            
    uint64_t last_set_value_{};    
    uint64_t const allowed_latency_{};
    uint64_t const cycle_length_{};
            
public:
        
    synchronization_object(gtl::d3d::command_queue&,                 // example:
                           unsigned max_value_in_cycle,              // If desired cycle is [0,1,2,3], max_value_in_cycle should be 3.
                           unsigned tolerated_difference_in_values); // If tolerated_diff is 2, the "completed" index value can lag behind
                                                                     //    the index value submitted to op() by 2; when it is greater, 
                                                                     //    the desync_call is executed instead (might be useful for logging..)                                                                     
    template <typename F, typename G>
    void operator()(F&& sync_call, G&& desync_call) {            
        if (values_are_synchronized()) {    
            sync_call(*this);
        } else {             
            desync_call();   
        }        
    }    
    
    ~synchronization_object();  // synchronizes

    synchronization_object(synchronization_object&&) = delete;
    synchronization_object& operator==(synchronization_object&&) = delete;

    friend unsigned value(synchronization_object const& s) { return s.periodic_value(); }
    friend void advance(synchronization_object& s) { s.advance(); }    

private:    
    
    void advance();
    unsigned periodic_value() const;    
    bool values_are_synchronized() const;
    void wait_for_values_to_sync_at(uint64_t new_value); // synchronizes
};


}}} // namespaces
#endif

