#ifndef KROWFOWAF_GTL_RELEASE_PTR_H_
#define KROWFOWAF_GTL_RELEASE_PTR_H_

/*-----------------------------------------------------------------------------
     Mikel Negugogor
        
     namespace gtl
     release_ptr<T> 

-----------------------------------------------------------------------------*/

#include <memory>
#include <stdexcept>
#include <iostream>

namespace gtl {
    
    namespace detail { struct release_deleter; }

    template <typename T, typename Deleter = ::gtl::detail::release_deleter>
    class release_ptr {        
        T* ptr_{};
    public:
        using type = T;

        release_ptr() = default;
        explicit release_ptr(T* ptr) noexcept : ptr_{ptr} {}

        release_ptr(release_ptr&& other) noexcept : ptr_{other.release()} {}        
        release_ptr& operator=(release_ptr&& other) noexcept { this->reset(other.release()); return *this; }

        ~release_ptr() { Deleter{}(ptr_); }
        
        void reset(T* t = nullptr) noexcept { Deleter{}(ptr_); ptr_ = t; }        

        T* release() noexcept { T* tmp = ptr_; ptr_ = nullptr; return tmp; }
        T* get() const noexcept { return ptr_; }

        // operator& overloading.. trying it out
        T** operator&() noexcept { return std::addressof(ptr_); }
        T*const* operator&() const noexcept { return std::addressof(ptr_); }
        //

        T*& expose_ptr() noexcept { reset(); return ptr_; } 
        
        T& operator*() const noexcept { return *ptr_; }
        T* operator->() const noexcept { return ptr_; }

        operator T*() const noexcept { return ptr_; }
        explicit operator bool() const noexcept { return ptr_ != nullptr; }
    };

    namespace detail {
        struct release_deleter {
            template <typename T>
            void operator()(T* t) const {
                if (t) t->Release(); 
            }
        };
    }

} // namespace gtl
#endif

