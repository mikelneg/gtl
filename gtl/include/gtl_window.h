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
#include <gtl/include/events.h>

namespace gtl {
namespace win {
    
    class window {                                               

        using wndproc_type = LRESULT(CALLBACK*)(HWND,UINT,WPARAM,LPARAM);

        HWND hwnd;                        
        MSG msg;
        std::pair<unsigned,unsigned> px_dims;
        std::vector<gtl::event> event_queue;        

        void pump_messages() {
            event_queue.clear();
            // the wndproc function is emplacing events in event_queue            
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }                                                        
        }

    public:                                                
                
        window(HINSTANCE inst, unsigned w_px, unsigned h_px, const char* caption);                
        window(window&&) = delete;
        window& operator=(window&&) = delete;
        
        // TODO handle cleanup..

        void post_close_message() const
        { 
            PostMessage(hwnd, WM_CLOSE, 0, 0); 
        }        

        void process_messages_and_swap_queues(std::vector<gtl::event>& swap_queue) {                                    
            pump_messages();
            swap_queue.swap(event_queue);            
        }
        
        ~window() {            
            if (*this) {
                std::cout << "Exited early..\n";
                post_close_message();
                while (*this) { 
                    pump_messages();
                    std::this_thread::yield();
                }
            }
        }

        operator bool () const { return msg.message != WM_QUIT; }

        friend unsigned width(window& w) noexcept { return w.px_dims.first; }
        friend unsigned height(window& w) noexcept { return w.px_dims.second; }
        friend HWND get_hwnd(window& w) noexcept { return w.hwnd; }
    };    

}} // namespaces
#endif


