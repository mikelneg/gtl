#ifndef OOWFIEFGGGF_GTL_WIN_TOOLS_H_
#define OOWFIEFGGGF_GTL_WIN_TOOLS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::win
    
    Various tools for MS Windows
-----------------------------------------------------------------------------*/

#include <windows.h>
#include <stdexcept>

namespace gtl {    
namespace win {
        
    constexpr bool succeeded(HRESULT result) noexcept { return result >= 0; }
    constexpr bool failed(HRESULT result) noexcept { return !succeeded(result); }

    inline void throw_on_fail(HRESULT result, const char* msg) { if (failed(result)) { throw std::runtime_error{msg}; } }

    constexpr auto width(RECT const& r) noexcept { return r.right - r.left; }
    constexpr auto height(RECT const& r) noexcept { return r.bottom - r.top; }

    template <typename R = UINT, typename T, int N>
    constexpr R array_size(T(&)[N]) { return static_cast<R>(N); }
    
    template <typename R = UINT, typename T>
    constexpr R array_size(std::initializer_list<T> list) { return static_cast<R>(list.size()); }

    class waitable_handle { 
        HANDLE handle_;
    public:        
        waitable_handle(HANDLE&& h) : handle_{h} { if (handle_ == nullptr){ throw std::runtime_error{__func__}; }}
        waitable_handle() : waitable_handle(CreateEvent(nullptr, FALSE, FALSE, nullptr)) {}
        waitable_handle(waitable_handle&&) = delete;
        waitable_handle& operator=(waitable_handle&&) = delete;
        ~waitable_handle() { CloseHandle(handle_); }
        friend void wait(waitable_handle const& h) { WaitForSingleObject(h.handle_, INFINITE); }
        operator HANDLE() { return handle_; }
    };

}} // namespaces
#endif

