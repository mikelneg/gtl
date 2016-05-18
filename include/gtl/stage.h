#ifndef RYUWOFWOABF_GTL_STAGE_H_
#define RYUWOFWOABF_GTL_STAGE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl
    class stage;
-----------------------------------------------------------------------------*/

#include <gtl/d3d_types.h>
#include <gtl/synchronization_object.h>

//#include <boost/coroutine/asymmetric_coroutine.hpp>
//#include <gtl/events.h>

#include <gtl/event_handler.h>
#include <gtl/rate_limiter.h>

#include <gtl/scene.h>
#include <gtl/command_variant.h>

//#include <vector>
//#include <array>

#include <thread>
#include <tuple>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>

#include <vn/tuple_utilities.h>

namespace gtl {

class stage {
    
    gtl::scene<gtl::command_variant const&> scene_;
    gtl::coroutine::event_handler event_handler_; 

    using pull_type = decltype(event_handler_)::pull_type;
    using push_type = decltype(event_handler_)::push_type;

    enum class sig {
        frame_consumed,
        frame_ready,        
    };
    
    std::thread work_thread_;
    std::mutex work_mutex_;
    std::condition_variable cv_;    
    bool quit_flag_; // the mutex guards this, so going with non-atomic
    std::atomic<sig> frame_state_;            

    gtl::rate_limiter frame_rate_limiter_;


    std::vector<ID3D12CommandList*> mutable draw_queue_;

    std::tuple<std::reference_wrapper<std::vector<ID3D12CommandList*>>,
               int,float,
               std::reference_wrapper<gtl::d3d::rtv_descriptor_heap>> draw_params_;            
    
    void work_thread(gtl::d3d::device, gtl::d3d::swap_chain&, 
                     gtl::d3d::command_queue&, unsigned);    

public:
    stage(gtl::d3d::swap_chain&, gtl::d3d::command_queue&, unsigned num_buffers);        
    ~stage();

    stage(stage&&) = delete;
    stage& operator=(stage&&) = delete;
    
    void present(gtl::d3d::swap_chain&,gtl::d3d::PresentParameters);           
    void dispatch_event(gtl::event const& e) { event_handler_.dispatch_event(e); }   
    
    gtl::scene<gtl::command_variant const&> 
    exchange_scene(gtl::scene<gtl::command_variant const&> s) {
        auto ret = std::exchange(scene_, std::move(s));
        scene_.send_command(gtl::commands::handle{});
        return ret;
    }

    template <typename F>                        
    void draw_callback(F func) const { vn::call_with_all_elements(draw_params_, f); }

    template <typename F>
    void replace_event_handler(F func) { event_handler_.replace_handler(std::move(f)); }        
};


} // namespace
#endif           
  
  
  
