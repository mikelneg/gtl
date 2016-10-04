/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef NBIWIWOWOWFF_GTL_RATE_LIMITER_H_
#define NBIWIWOWOWFF_GTL_RATE_LIMITER_H_

#include <chrono>
#include <thread>

namespace gtl {

class rate_limiter {
    using clock = std::chrono::high_resolution_clock;

    using time_point = clock::time_point;
    using duration = clock::duration;

    duration const event_duration_;
    time_point mutable last_time_;
    time_point mutable begin_time_of_next_;    

public:
    
    rate_limiter(duration min_duration_between_invocations)
        : event_duration_{min_duration_between_invocations}, 
          last_time_{clock::now()},
          begin_time_of_next_{last_time_ + event_duration_}
    {}

    template <typename F>
    void skip_or_invoke(time_point current_time, F&& func) const 
    {
        if (current_time < begin_time_of_next_) { 
            return; 
        }            
        begin_time_of_next_ = current_time + event_duration_;
        func(current_time - last_time_);
        last_time_ = current_time;
    }

    template <typename F>
    void sleepy_invoke(time_point current_time, F&& func) const
    {                                
        if (current_time < begin_time_of_next_) {               // if we arrive early and haven't reached the next execution window..
            std::this_thread::sleep_until(begin_time_of_next_); // sleep it off..                                    
            current_time = begin_time_of_next_;
        }                
        
        begin_time_of_next_ = current_time + event_duration_;   // then advance our next execution window by one event_duration..                            
        func(current_time - last_time_);
        last_time_ = current_time;        
    }

    template <typename F>
    inline void skip_or_invoke(F&& func) const 
    {
        skip_or_invoke(clock::now(),std::forward<F>(func)); 
    }

    template <typename F>
    inline void sleepy_invoke(F&& func) const
    {                        
        sleepy_invoke(clock::now(),std::forward<F>(func));
    }
    
};


} // namespace
#endif