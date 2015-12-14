#include "../include/gtl_window.h"
#include "../include/keyboard_enum.h"
#include "../include/system_caps.h"

#include <gtl/include/event.h>
#include <vector>

#include <iostream> // debugging
#include <thread>
#include <utility>
#include <windows.h>
#include <windowsx.h>
#include <stdexcept>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace win {

namespace {

    template <typename HandlerType>
    static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

    static WNDCLASSEX default_wndclassex() noexcept;
           
    static void resize_window(HWND hwnd, unsigned width, unsigned height);

    static HWND CreateFullscreenWindow(HWND hwnd, HINSTANCE hinst, const char* class_name, const char* caption, void* sneaky)
    {   // adapted from Raymond Chen: http://blogs.msdn.com/b/oldnewthing/archive/2005/05/05/414910.aspx
        HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (!GetMonitorInfo(hmon, &mi)) return NULL;
        return CreateWindow(class_name,
              caption,
              WS_POPUP | WS_VISIBLE,
              mi.rcMonitor.left,
              mi.rcMonitor.top,
              mi.rcMonitor.right - mi.rcMonitor.left,
              mi.rcMonitor.bottom - mi.rcMonitor.top,
              hwnd, NULL, hinst, sneaky);
    }
}
    
window::window(HINSTANCE hinstance, unsigned width_px, unsigned height_px, const char* caption, std::vector<gtl::event>& event_queue)
: px_dims{width_px,height_px}, 
  hwnd{}
{
  auto style = default_wndclassex();
  auto func = &wndproc<std::vector<gtl::event>>;
  style.lpfnWndProc = func;
  style.hInstance = hinstance;        
  style.lpszClassName = u8"window";    

  if (RegisterClassEx(&style) == 0) { // failure returns 0
      throw std::runtime_error{__func__};
  }
  //hwnd = CreateFullscreenWindow(hwnd,hinstance,style.lpszClassName,caption,this);
  hwnd = CreateWindow(style.lpszClassName, caption, 
                      WS_VISIBLE | WS_POPUP,
                      0, 0, // dummy positions, adjusted later with ResizeWindow()
                      0, 0, // dummy width and height, adjusted later with ResizeWindow()
                      nullptr, nullptr, // hWndParent and hMenu
                      hinstance,      
                      std::addressof(event_queue)); // how "this" gets smuggled into wnd_proc()
  if (!hwnd) {
      throw std::runtime_error{__func__};   
  }          
  resize_window(hwnd, width_px, height_px);     // remove for fullscreen                
}   


namespace {
    static WNDCLASSEX default_wndclassex() noexcept
    {
        WNDCLASSEX config_{};
        config_.cbSize = sizeof(WNDCLASSEX);
        config_.style = CS_HREDRAW | CS_VREDRAW;
        config_.hCursor = LoadCursor(NULL, IDC_ARROW);
        return config_;
    }    

    static void resize_window(HWND hwnd, unsigned width_px, unsigned height_px)
    {        
        RECT client_rect_{};
        RECT window_rect_{};
        
        GetClientRect(hwnd, &client_rect_);
        GetWindowRect(hwnd, &window_rect_);        
                       
        int dx = static_cast<int>((width_px - client_rect_.right) / 2);
        int dy = static_cast<int>((height_px - client_rect_.bottom) / 2);
    
        InflateRect(&window_rect_, dx, dy);
        SetWindowPos(hwnd, HWND_TOP, 200, 0,
            window_rect_.right - window_rect_.left,
            window_rect_.bottom - window_rect_.top,
            SWP_SHOWWINDOW | SWP_NOZORDER);
    }
    

/////////////////////////////////////////////////////////////////////////////

    template <typename EventQueue>
    static 
    LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // CreateWindowEx() in Window ctor calls this function several times before 
        // returning (with WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_CREATE
        // "lparam" contains the last parameter passed in the CreatqeWindowEx() call. 
        // We extract that pointer here and keep a copy to dispatch messages..
        // When ~Window() is called this function is called with WM_DESTROY (before destroyed)
        // and WM_NCDESTROY (after destroyed), and so we null the pointer then.. 
        
        // We keep a non-owning pointer to our queue object, initialized with WM_CREATE, nulled again with WM_NCDESTROY        
        thread_local EventQueue *ev_queue{};    // TODO check emitted code to see how this is handled..

        using gtl::event_types::keydown;
        using gtl::event_types::sendquit;

        switch (msg) {
            case WM_KEYUP: break;                                   
            case WM_KEYDOWN: if ((HIWORD(lparam) & KF_REPEAT) < 1) { 
                                 // I am not checking ev_queue before dereferencing it because I am assuming
                                 // initialization has gone as planned.. 
                ev_queue->emplace_back(gtl::event{keydown{static_cast<unsigned>(wparam)}});  
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
            
            case WM_NCCREATE: ev_queue = reinterpret_cast<EventQueue*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);                              
                              break;            

            case WM_NCCALCSIZE: break; // this_ might be nullptr; do not touch!    
            case WM_GETMINMAXINFO: break; // this_ is probably nullptr; do not touch!    
            case WM_KILLFOCUS: break;    
            case WM_SETFOCUS: SetFocus(hwnd); break;            

            case WM_CLOSE: DestroyWindow(hwnd);
                           break;

            case WM_DESTROY: PostQuitMessage(0); // Beginning of destruction                             
                             break;

            case WM_NCDESTROY: // End of destruction, should be last call to msg_proc..
                               ev_queue = nullptr; // should be final call to this msg_proc                                                              
                               break;             

            case WM_SIZE: break;    
            case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE); // disable beep on alt-enter     
            
            default: break; 
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

} // anonymous namespace
}} // namespaces
