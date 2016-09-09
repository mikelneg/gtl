#ifndef KROWFOWAF_GTL_INTRUSIVE_PTR_H_
#define KROWFOWAF_GTL_INTRUSIVE_PTR_H_

/*-----------------------------------------------------------------------------
     Mikel Negugogor
             
     class gtl::intrusive_ptr<T,Cleanup> 

     A non-owning pointer type that calls Cleanup(ptr) (optionally Startup(ptr) as well..) 

     * Cleanup(ptr) is called only if ptr is non-null
     * Startup(ptr) is called only if Startup is passed in constructor

-----------------------------------------------------------------------------*/

#include <memory>
#include <utility>

namespace gtl {

template <typename T, typename Cleanup>
class intrusive_ptr : private Cleanup {

    T* ptr_{ nullptr };

public:
    using type = T;

    intrusive_ptr() = default;

    template <typename Startup>
    explicit intrusive_ptr(T* ptr, Startup&& startup, Cleanup&& c)
        : Cleanup(std::move(c))
        , ptr_{ ptr }
    {
        startup(ptr_);
    }

    explicit intrusive_ptr(T* ptr, Cleanup&& c) noexcept
        : Cleanup(std::move(c)),
          ptr_{ ptr }
    {
    }

    explicit intrusive_ptr(T* ptr) noexcept
        : ptr_{ ptr }
    {
    }

    intrusive_ptr(intrusive_ptr&& other) noexcept
        : Cleanup(std::move(static_cast<Cleanup&>(other))),
          ptr_{ other.release() }
    {
    }

    intrusive_ptr& operator=(intrusive_ptr other) noexcept
    {
        this->swap(other);
        return *this;
    }

    ~intrusive_ptr()
    {
        static_assert(noexcept(this->Cleanup::operator()(ptr_)), "intrusive_ptr<T,Cleanup>: Cleanup::operator()(T*) must be noexcept.");
        this->reset();
    }

    T* release() noexcept { return std::exchange(ptr_, nullptr); } // no Cleanup

    void reset(T* t = nullptr) noexcept
    { // Cleanup if ptr_ is non-null
        if (ptr_) {
            Cleanup::operator()(ptr_);
            ptr_ = t;
        }
    }

    T*& expose_ptr() noexcept
    {
        this->reset();
        return ptr_;
    } // Cleanup; resets internal pointer and returns it by reference

    T* get() const noexcept { return ptr_; } // same const semantics as std::unique_ptr
    T& operator*() const noexcept { return *ptr_; } // ...
    T* operator->() const noexcept { return ptr_; } // ...

    operator T*() const noexcept { return ptr_; } // T* conversion operator
    explicit operator bool() const noexcept { return ptr_ != nullptr; } // explicit bool conversion

    //

    void swap(intrusive_ptr& other)
    {
        using std::swap;
        swap(static_cast<Cleanup&>(*this), static_cast<Cleanup&>(other));
        swap(ptr_, other.ptr_);
    }

    // adl friend functions

    friend void swap(intrusive_ptr& a, intrusive_ptr& b)
    {
        a.swap(b);
    }

    friend void** expose_as_void_pp(intrusive_ptr& t) noexcept
    {
        return std::addressof(reinterpret_cast<void*&>(t.expose_ptr()));
        //return reinterpret_cast<void**>(std::addressof(t.expose_ptr()));
    }

    friend T*& expose(intrusive_ptr& t) noexcept
    { // Cleanup; resets internal pointer and returns it by reference
        return t.expose_ptr();
    }
};

} // namespace gtl
#endif
