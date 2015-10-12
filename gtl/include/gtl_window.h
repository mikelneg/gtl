#ifndef YWIWVNWMASDF_GTL_WINDOW_H_
#define YWIWVNWMASDF_GTL_WINDOW_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::window;           
-----------------------------------------------------------------------------*/

#include <cstddef>
#include <windows.h>
#include <utility>

namespace gtl {
inline namespace win {

    class window {                       
        HWND hwnd;
        std::pair<size_t,size_t> px_dims;        

        window(window&&) = delete;
        window& operator=(window&&) = delete;

    public:                                
        window(HINSTANCE, size_t width_px, size_t height_px, const char*);             
       ~window();                         

        friend size_t width(window& w) noexcept { return w.px_dims.first; }
        friend size_t height(window& w) noexcept { return w.px_dims.second; }
        friend HWND get_hwnd(window& w) noexcept { return w.hwnd; }
    };

}} // namespaces
#endif


