#ifndef YWIWVNWMASDF_GTL_WINDOW_H_
#define YWIWVNWMASDF_GTL_WINDOW_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::window;           
-----------------------------------------------------------------------------*/

#include <cstddef>
#include <windows.h>
#include <utility>
#include <gtl/include/event.h>

namespace gtl {
namespace win {
    
    class window {                                               
                
        HWND hwnd;
        std::pair<unsigned,unsigned> px_dims;        
                
    public:                                                
        
        window(HINSTANCE hinstance, unsigned width_px, unsigned height_px, 
               const char* caption, std::vector<gtl::event>& event_queue);
        
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


