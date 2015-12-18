#ifndef BOAWASPOAFEFA_GTL_WINDOW_WNDPROC_H_
#define BOAWASPOAFEFA_GTL_WINDOW_WNDPROC_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::win::detail

    wndproc_impl
-----------------------------------------------------------------------------*/

#include <windows.h>
#include <gtl/include/events.h>

namespace gtl {
namespace win {
namespace detail {

    template <typename T>    
    LRESULT CALLBACK wndproc_impl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // CreateWindowEx() in Window ctor calls this function several times before 
        // returning (with WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_CREATE
        // "lparam" contains the last parameter passed in the CreatqeWindowEx() call. 
        // We extract that pointer here and keep a copy to dispatch messages..
        // When ~Window() is called this function is called with WM_DESTROY (before destroyed)
        // and WM_NCDESTROY (after destroyed), and so we null the pointer then.. 
        
        // We keep a non-owning pointer to our queue object, initialized with WM_CREATE, nulled again with WM_NCDESTROY        
        static T *event_handler{};    // TODO check emitted code to see how this is handled..

        using gtl::events::keydown;
        using gtl::events::sendquit;   

        switch (msg) {
            case WM_KEYUP: break;                                   
            case WM_KEYDOWN: if ((HIWORD(lparam) & KF_REPEAT) < 1) { 
                                 // I am not checking before dereferencing because I am assuming
                                 // initialization has gone as planned.. 
                                dispatch_event(*event_handler,keydown{static_cast<unsigned>(wparam)});    
                             } 
                             break;            
            case WM_SYSKEYDOWN: break;            
            case WM_MOUSEWHEEL: break;     
            case WM_MOUSEMOVE:  break;    
            case WM_LBUTTONDOWN: break;            
            case WM_CAPTURECHANGED: return 0;            
            case WM_LBUTTONUP: break;
            
            //----------------------------------//                 
            case WM_SETCURSOR: if (LOWORD(lparam) == HTCLIENT) {
                                        SetCursor(NULL);
                                        return TRUE;
                                   } 
                                   break;    
            
            case WM_NCCREATE: event_handler = reinterpret_cast<decltype(event_handler)>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);                              
                              break;            

            case WM_NCCALCSIZE: break; // this_ might be nullptr; do not touch!    
            case WM_GETMINMAXINFO: break; // this_ is probably nullptr; do not touch!    
            case WM_KILLFOCUS: std::cout << "losing focus..\n"; break;    
            case WM_SETFOCUS: std::cout << "focus returned..\n"; SetFocus(hwnd); break;            

            case WM_CLOSE: DestroyWindow(hwnd);
                           break;

            case WM_DESTROY: PostQuitMessage(0); // Beginning of destruction                             
                             break;

            case WM_NCDESTROY: // End of destruction, should be last call to msg_proc..
                               event_handler = nullptr; // should be final call to this msg_proc                                                              
                               break;             

            case WM_SIZE: break;    
            case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE); // disable beep on alt-enter     
            
            default: break; 
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }        

}}} // namespaces
#endif