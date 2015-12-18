#ifndef YWIWVNWMASDF_GTL_WINDOW_H_
#define YWIWVNWMASDF_GTL_WINDOW_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::window;           
-----------------------------------------------------------------------------*/

#include <windows.h>
#include <gtl/include/gtl_window_wndproc.h>

namespace gtl {
namespace win {
    
    class window {                                               

        using wndproc_type = LRESULT(CALLBACK*)(HWND,UINT,WPARAM,LPARAM);

        HWND hwnd;                        
        std::pair<unsigned,unsigned> px_dims;
        
        window(HINSTANCE, unsigned, unsigned, const char*, wndproc_type, void*);

    public:                                                
        
        template <typename T>
        window(HINSTANCE inst, unsigned w_px, unsigned h_px, const char* cap, T& msg_handler) 
        :   window(inst, w_px, h_px, cap, 
                   std::addressof(detail::wndproc_impl<T>), 
                   std::addressof(msg_handler)) 
        {}
        
        window(window&&) = delete;
        window& operator=(window&&) = delete;
        
        // TODO handle cleanup..

        void post_close_message() const
        { 
            PostMessage(hwnd, WM_CLOSE, 0, 0); 
        }

        friend unsigned width(window& w) noexcept { return w.px_dims.first; }
        friend unsigned height(window& w) noexcept { return w.px_dims.second; }
        friend HWND get_hwnd(window& w) noexcept { return w.hwnd; }
    };    

}} // namespaces
#endif


