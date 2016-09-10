/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YWWIWOFOJWF_GTL_LOCKSWAP_VECTOR_H_
#define YWWIWOFOJWF_GTL_LOCKSWAP_VECTOR_H_

#include <memory>
#include <mutex>
#include <vector>
#include <vn/boost_variant_utilities.h>

namespace gtl {

template <typename T, typename A = std::allocator<T>>
class lockswap_vector {
    using queue_type = std::vector<T, A>;

    queue_type queue_;
    std::mutex mutex_;

public:
    lockswap_vector() = default;
    lockswap_vector(lockswap_vector&&) = delete;
    lockswap_vector& operator=(lockswap_vector&&) = delete;

    void swap_out(queue_type& other)
    { // clears other before swap(other,this->queue_);
        other.clear();
        std::lock_guard<std::mutex> lock{mutex_};
        queue_.swap(other);
    }

    void push(T&& t)
    {
        queue_.emplace_back(std::move(t));
    }
    queue_type make_lockswap_vector(size_t initial_reserve = 0)
    {
        queue_type q;
        q.reserve(initial_reserve);
        return q;
    }
};

} // namespaces
#endif
