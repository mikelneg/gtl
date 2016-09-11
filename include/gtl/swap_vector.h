/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef IRKKOWOWLSFE_GTL_SWAP_VECTOR_H_
#define IRKKOWOWLSFE_GTL_SWAP_VECTOR_H_

#include <atomic>
#include <memory>
#include <vector>

namespace gtl {

template <typename T, typename A = std::allocator<T>>
class swap_vector {
    using vector_type = std::vector<T, A>;

    enum class status { stale, fresh, busy };

    vector_type vector_;
    std::atomic<status> status_flag_;

public:
    swap_vector() : status_flag_{status::stale}
    {
    }

    swap_vector(vector_type v_) : vector_(std::move(v_)), status_flag_{status::fresh}
    {
    }

    swap_vector(swap_vector const&) = delete;
    swap_vector& operator=(swap_vector const&) = delete;

    vector_type make_vector() const
    {
        vector_type v;
        return v;
    }

    void swap_in(vector_type& v)
    {

        status test_flag_{status::fresh};

        while (!status_flag_.compare_exchange_weak(test_flag_, status::busy, std::memory_order_release))
        {
            if (test_flag_ == status::busy)
            {
                test_flag_ = status::fresh;
            }
        }
        vector_.swap(v);
        status_flag_.store(status::fresh, std::memory_order_release);
    }

    void copy_in(vector_type const& v)
    {

        status test_flag_{status::fresh};

        while (!status_flag_.compare_exchange_weak(test_flag_, status::busy, std::memory_order_release))
        {
            if (test_flag_ == status::busy)
            {
                test_flag_ = status::fresh;
            }
        }
        vector_ = v;
        status_flag_.store(status::fresh, std::memory_order_release);
    }

    vector_type copy_out()
    {
        status test_flag_{status::fresh};

        while (!status_flag_.compare_exchange_weak(test_flag_, status::busy, std::memory_order_release))
        {
            if (test_flag_ == status::busy)
            {
                test_flag_ = status::fresh;
            }
        }
        vector_type v = vector_;
        status_flag_.store(status::stale, std::memory_order_release);
        return v;
    }

    bool swap_out(vector_type& v)
    {
        status test_flag_{status::fresh};
        if (status_flag_.compare_exchange_strong(test_flag_, status::busy, std::memory_order_release))
        {
            vector_.swap(v);
            status_flag_.store(status::stale, std::memory_order_release);
            return true;
        }
        // else
        return false;
    }
};

} // namespace
#endif