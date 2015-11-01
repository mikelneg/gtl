#include "../include/gtl_window.h"
#include "../include/win_keyboard_enum.h"
#include "../include/system_caps.h"

#include <iostream> // debugging
#include <thread>
#include <windows.h>
#include <windowsx.h>
#include <stdexcept>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace win {

    namespace {
        static WNDCLASSEX default_wndclassex() noexcept;
        static LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);
        static void resize_window(HWND hwnd, size_t width, size_t height);

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
    
    window::window(HINSTANCE hinstance, size_t width_lpx, size_t height_lpx, const char* caption)
        : px_dims{width_lpx,height_lpx}, 
          hwnd{}
    {
        auto style = default_wndclassex();
        style.lpfnWndProc = &wnd_proc;
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
                            this); // how "this" gets smuggled into wnd_proc()
        if (!hwnd) {
            throw std::runtime_error{__func__};   
        }        

        //SetCursor(NULL);
        resize_window(hwnd, width_lpx, height_lpx);     // remove for fullscreen                
    }    
        
    window::~window()
    {   
       // if (hwnd_) {
       //     DestroyWindow(hwnd_); // sends WM_DESTROY to static_msg_proc()
       // }
    }
    
    namespace {

    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // CreateWindowEx() in Window ctor calls this function several times before 
        // returning (with WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_CREATE
        // "lparam" contains the last parameter passed in the CreatqeWindowEx() call. 
        // We extract that pointer here and keep a copy to dispatch messages..
        // When ~Window() is called this function is called with WM_DESTROY (before destroyed)
        // and WM_NCDESTROY (after destroyed), and so we null the pointer then.. 
        static window* gtl_ptr{nullptr}; // non-owning, set with WM_CREATE, nulled with WM_NCDESTROY
    
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
                              PostQuitMessage(0);
                              break;
    
            case WM_NCDESTROY: gtl_ptr = nullptr; // should be final call to this msg_proc                               
                               break; 
            
            case WM_SIZE:     break;
    
            case WM_MENUCHAR: // disable beep on alt-enter 
                              return MAKELRESULT(0, MNC_CLOSE);
    
            default:          break;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    
    static WNDCLASSEX default_wndclassex() noexcept
    {
        WNDCLASSEX config_{};
        config_.cbSize = sizeof(WNDCLASSEX);
        config_.style = CS_HREDRAW | CS_VREDRAW;
        config_.hCursor = LoadCursor(NULL, IDC_ARROW);
        return config_;
    }    

    static void resize_window(HWND hwnd, size_t width_px, size_t height_px)
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
    }
}} // namespaces
