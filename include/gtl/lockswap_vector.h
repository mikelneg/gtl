#ifndef YWWIWOFOJWF_GTL_LOCKSWAP_VECTOR_H_
#define YWWIWOFOJWF_GTL_LOCKSWAP_VECTOR_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    
    class lockswap_vector;
-----------------------------------------------------------------------------*/

#include <vector>
#include <mutex>
#include <memory>
#include <vn/boost_utilities.h>

namespace gtl {

template <typename T, typename A = std::allocator<T>>
class lockswap_vector {        
    using queue_type = std::vector<T,A>;

    queue_type queue_;
    std::mutex mutex_;

public:
    lockswap_vector() = default;
    lockswap_vector(lockswap_vector&&) = delete;
    lockswap_vector& operator=(lockswap_vector&&) = delete;
        
    void swap_out(queue_type& other) { // clears other before swap(other,this->queue_);
        other.clear();
        std::lock_guard<std::mutex> lock{mutex_};        
        queue_.swap(other);
    }

    void push(T&& t) { queue_.emplace_back(std::move(t)); }
    queue_type make_lockswap_vector(size_t initial_reserve = 0) { queue_type q; q.reserve(initial_reserve); return q; }
};



} // namespaces
#endif
