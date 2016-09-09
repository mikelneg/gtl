#ifndef NBIWIWOWOWFF_GTL_RATE_LIMITER_H_
#define NBIWIWOWOWFF_GTL_RATE_LIMITER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl
    class rate_limiter;
-----------------------------------------------------------------------------*/

#include <chrono>

namespace gtl {

class rate_limiter {
    using clock = std::chrono::high_resolution_clock;

    using time_point = clock::time_point;
    using duration = clock::duration;

    duration const event_duration_;
    time_point mutable begin_time_of_next_;

public:
    rate_limiter(std::chrono::milliseconds m)
        : event_duration_{ std::chrono::duration_cast<duration>(m) }
        , begin_time_of_next_{ clock::now() + event_duration_ }
    {
    }

    template <typename F>
    void operator()(F&& func) const
    {
        auto current_time = clock::now();
        if (current_time < begin_time_of_next_) {
            return;
        }
        begin_time_of_next_ = current_time + event_duration_;
        func();
    }
};

} // namespace
#endif