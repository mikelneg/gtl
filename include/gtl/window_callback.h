#ifndef UOWGIHBOWPWF_GTL_WINDOW_CALLBACK_H_
#define UOWGIHBOWPWF_GTL_WINDOW_CALLBACK_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl::win::    
    class window_callback; 
-----------------------------------------------------------------------------*/
#include <Windows.h>

namespace gtl {
namespace win {


        

class callback {

    template <typename T>
    static LRESULT CALLBACK wndproc(HWND,UINT,WPARAM,LPARAM);

public:
    template <typename EventQueue> 
    callback(EventQueue&);
};


template <typename EventQueue>
static LRESULT CALLBACK callback::wndproc(HWND,UINT,WPARAM,LPARAM) 
{
        // CreateWindowEx() in Window ctor calls this function several times before 
        // returning (with WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_CREATE
        // "lparam" contains the last parameter passed in the CreatqeWindowEx() call. 
        // We extract that pointer here and keep a copy to dispatch messages..
        // When ~Window() is called this function is called with WM_DESTROY (before destroyed)
        // and WM_NCDESTROY (after destroyed), and so we null the pointer then.. 

        static EventQueue* event_queue{nullptr}; // non-owning, set with WM_CREATE, nulled with WM_NCDESTROY
    
        switch (msg) {
            case WM_KEYUP:    break;                               
    
            case WM_KEYDOWN:  if ((HIWORD(lparam) & KF_REPEAT) < 1) {
                                if (wparam == keyboard::Q) {
                                    PostMessage(hwnd, WM_CLOSE, 0, 0); 
                                } else {                                
                                    std::cout << "Key pressed: thread is " << std::this_thread::get_id() << "\n";
                                }
                              }
                              break;
            
            case WM_SYSKEYDOWN: 
                                break;
            
            case WM_MOUSEWHEEL: 
                                break; 
    
            case WM_MOUSEMOVE:  break;
    
            case WM_LBUTTONDOWN: 
                                 break;
            
            case WM_CAPTURECHANGED: return 0;
            
            case WM_LBUTTONUP:  break;
            
            //----------------------------------//
    
            // case WM_QUIT: // WM_QUIT is not associated with a window and never sent to static_msg_proc()..
    
            case WM_SETCURSOR: if (LOWORD(lparam) == HTCLIENT) {
                                    SetCursor(NULL);
                                    return TRUE;
                               } 
                               break;
    
            case WM_NCCREATE: gtl_ptr = reinterpret_cast<window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);                              
                              break;
            
            case WM_NCCALCSIZE: break; // this_ might be nullptr; do not touch!
    
            case WM_GETMINMAXINFO: break; // this_ is probably nullptr; do not touch!
    
            case WM_KILLFOCUS: 
                               break;
    
            case WM_SETFOCUS:  
                              break;
            
            case WM_CLOSE:    std::cout << "WM_CLOSE recvd. \n";                              
                              //PostQuitMessage(0);
                              DestroyWindow(hwnd);
                              break;

            case WM_DESTROY : // Beginning of destruction
                              PostQuitMessage(0);
                              break;

            case WM_NCDESTROY:  // End of destruction, should be last call to msg_proc..
                               gtl_ptr = nullptr; // should be final call to this msg_proc                                                              
                               break; 
            
            case WM_SIZE:     break;
    
            case WM_MENUCHAR: // disable beep on alt-enter 
                              return MAKELRESULT(0, MNC_CLOSE);
    
            default:          break;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }


}} // namespaces
#endif
