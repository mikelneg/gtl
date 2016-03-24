#ifndef UWOHGBASFF_GTL_WNDPROC_H_
#define UWOHGBASFF_GTL_WNDPROC_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::win::
    
    wndproc function 
-----------------------------------------------------------------------------*/

#include <Windows.h>
#include <memory>
#include <functional>
#include <gtl/events.h>

namespace gtl {
namespace win {
    
    template <typename E>    
    class wndproc {        
        template <typename HandlerType>
        static LRESULT CALLBACK wndproc_func(HWND, UINT, WPARAM, LPARAM);

    public:

        using wndproc_type = LRESULT(CALLBACK *)(HWND,UINT,WPARAM,LPARAM);                
        using handler_type = std::function<void(E)>;
        
        handler_type const handler;
        
        template <typename H>
        wndproc(std::function<void(E)> handler) : handler{std::move(handler)} {}  

        wndproc_type ptr() const { return &wndproc_func<handler_type>; }
        handler_type handler() const { return }        
    };
    
    template <typename E>
    template <typename HandlerType>
    LRESULT CALLBACK wndproc<E>::wndproc_func(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // CreateWindowEx() in Window ctor calls this function several times before 
        // returning (with WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_CREATE
        // "lparam" contains the last parameter passed in the CreatqeWindowEx() call. 
        // We extract that pointer here and keep a copy to dispatch messages..
        // When ~Window() is called this function is called with WM_DESTROY (before destroyed)
        // and WM_NCDESTROY (after destroyed), and so we null the pointer then.. 
        thread_local HandlerType msg_handler; // begins as a dummy; sets with WM_CREATE, nulled with WM_NCDESTROY
    
        switch (msg) {
            case WM_KEYUP: break;                                   
            case WM_KEYDOWN: if ((HIWORD(lparam) & KF_REPEAT) < 1) { 
                                 msg_handler(gtl::events::keydown{wparam});  
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
            
            case WM_NCCREATE: msg_handler = *reinterpret_cast<HandlerType*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);                              
                              break;            

            case WM_NCCALCSIZE: break; // this_ might be nullptr; do not touch!    
            case WM_GETMINMAXINFO: break; // this_ is probably nullptr; do not touch!    
            case WM_KILLFOCUS: break;    
            case WM_SETFOCUS: break;            

            case WM_CLOSE: DestroyWindow(hwnd);
                           break;

            case WM_DESTROY: PostQuitMessage(0); // Beginning of destruction
                             break;

            case WM_NCDESTROY: // End of destruction, should be last call to msg_proc..
                               //handler = [](auto){}; // should be final call to this msg_proc                                                              
                               break;             

            case WM_SIZE: break;    
            case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE); // disable beep on alt-enter     
            
            default: break; 
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }




}} // namespaces
#endif
