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
        
    constexpr bool succ(HRESULT result) noexcept { return result >= 0; }
    constexpr bool fail(HRESULT result) noexcept { return !succ(result); }

    inline void throw_on_fail(HRESULT result, const char* msg) { if (fail(result)) { throw std::runtime_error{msg}; } }

    constexpr auto width(RECT const& r) noexcept { return r.right - r.left; }
    constexpr auto height(RECT const& r) noexcept { return r.bottom - r.top; }

    std::array<int,5>;

    
}} // namespaces
#endif

