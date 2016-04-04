#ifndef YWIWVNWMASDF_GTL_WINDOW_H_
#define YWIWVNWMASDF_GTL_WINDOW_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::window;           
-----------------------------------------------------------------------------*/

#include <windows.h>
#include <vector>
#include <iostream> // TODO debugging
#include <thread>
#include <gtl/events.h>

namespace gtl {
namespace win {
    
    class window {                                               

        using wndproc_type = LRESULT(CALLBACK*)(HWND,UINT,WPARAM,LPARAM);

        HWND hwnd;                        
        MSG msg;
        std::pair<unsigned,unsigned> px_dims;
        std::vector<gtl::event> event_queue;        

        void extract_messages() {            
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg); // wndproc is emplacing events in event_queue            
            }                                                        
        }

        // void get_messages() {            
        //    BOOL ret;
        //    while((ret = GetMessage( &msg, NULL, 0, 0 )) != 0)
        //    { 
        //        if (ret == -1)
        //        {
        //            // error case..
        //        }
        //        else
        //        {
        //            TranslateMessage(&msg); 
        //            DispatchMessage(&msg); 
        //        }
        //    }
        // }

    public:                                                
                
        window(HINSTANCE hinst, unsigned w_px, unsigned h_px, const char* caption);                
        window(window&&) = delete;
        window& operator=(window&&) = delete;                            
        
        template <typename F>
        void pump_messages(F func) {            
            while (true) {
                event_queue.clear();
                extract_messages();
                if (msg.message == WM_QUIT) { 
                    return; 
                } 
                func(const_cast<decltype(event_queue) const&>(event_queue));
            }
        }
        
        ~window() {            
            if (*this) { // if msg.message != WM_QUIT then we have exited early..                                
                DestroyWindow(hwnd);  // WM_DESTROY should issue PostQuitMessage(0);
                pump_messages([](auto&){ std::this_thread::yield(); }); // pump until WM_QUIT                
            }
        }

        operator bool () const { return msg.message != WM_QUIT; }

        friend unsigned width(window& w) noexcept { return w.px_dims.first; }
        friend unsigned height(window& w) noexcept { return w.px_dims.second; }
        friend HWND get_hwnd(window& w) noexcept { return w.hwnd; }
    };    

}} // namespaces
#endif


