/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef OOWFIEFGGGF_GTL_WIN_TOOLS_H_
#define OOWFIEFGGGF_GTL_WIN_TOOLS_H_

#include <stdexcept>
#include <windows.h>

namespace gtl {
namespace win {

    constexpr bool success(HRESULT result) noexcept
    {
        return result >= 0;
    } // we hate macros; also, constexpr implies inline
    constexpr bool fail(HRESULT result) noexcept
    {
        return !success(result);
    }

    template <typename Exception = std::runtime_error>
    inline void throw_on_fail(HRESULT result, const char* msg)
    {
        if (fail(result))
        {
            throw Exception{msg};
        }
    }

    constexpr auto width(RECT const& r) noexcept
    {
        return r.right - r.left;
    }
    constexpr auto height(RECT const& r) noexcept
    {
        return r.bottom - r.top;
    }

    template <typename R = UINT, typename T, int N>
    constexpr R extent(T (&)[N]) noexcept
    {
        return static_cast<R>(N);
    }

    template <typename R = UINT, typename T>
    constexpr R extent(std::initializer_list<T> list) noexcept
    {
        return static_cast<R>(list.size());
    }

    template <typename R = UINT, typename T>
    constexpr R extent(T const& obj)
    {
        return static_cast<R>(obj.size());
    }

    class waitable_handle {
        HANDLE handle_;

    public:
        waitable_handle() : handle_(CreateEvent(nullptr, FALSE, FALSE, nullptr))
        {
            if (handle_ == NULL)
                throw std::runtime_error{__func__};
        }
        waitable_handle(waitable_handle&&) = delete;            // no moving
        waitable_handle& operator=(waitable_handle&&) = delete; // no assigning
        ~waitable_handle()
        {
            CloseHandle(handle_); // note: CloseHandle() will throw in debug mode if issued on an invalid handle..
        }

        operator HANDLE()
        {
            return handle_;
        }

        void wait() const
        {
            WaitForSingleObject(handle_, INFINITE);
        }
        friend void wait(waitable_handle const& h)
        {
            h.wait();
        }
    };
}
} // namespaces
#endif
