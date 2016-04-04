#ifndef NBUWABVASDFFW_GTL_COPYABLE_ATOMIC_H_
#define NBUWABVASDFFW_GTL_COPYABLE_ATOMIC_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    
    template <typename T>
    struct copyable_atomic;

        - an atomic with default constructor, copy constructor and assignment operator.
        - uses release/acquire semantics 
-----------------------------------------------------------------------------*/

#include <atomic>

namespace gtl {

    template <typename T>
    struct copyable_atomic {
        std::atomic<T> data;
        copyable_atomic() noexcept { data.store(T{},std::memory_order_relaxed); }
        copyable_atomic(copyable_atomic const& other) noexcept { *this = other; }
        copyable_atomic& operator=(copyable_atomic const& other) noexcept { 
            data.store(other.data.load(std::memory_order_acquire),std::memory_order_release); 
            return *this; }

        T get() const { return data.load(std::memory_order_acquire); }
        void set(T t) { data.store(t,std::memory_order_release); }
    };

} // namespace
#endif
