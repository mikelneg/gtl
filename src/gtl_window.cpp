#include "gtl/gtl_window.h"

#include <gtl/keyboard_enum.h>
#include <gtl/system_caps.h>

#include <gtl/gtl_window_wndproc.h>
#include <vn/boost_coroutine_utilities.h>
//#include <iostream> // debugging
//#include <thread>
#include <utility>
#include <windows.h>
#include <windowsx.h>
#include <stdexcept>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace win {

// impl details
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
      


} // unnamed namespace 


    
window::window(HINSTANCE hinstance, unsigned width_px, unsigned height_px, const char* caption)
    :   hwnd{}, 
        msg{},
        width_px{width_px},
        height_px{height_px},
        event_queue_{}
{  
  auto style = default_wndclassex();
  auto func = &gtl::win::detail::wndproc_impl<decltype(event_queue_)>;
  style.lpfnWndProc = func;
  style.hInstance = hinstance;        
  style.lpszClassName = u8"window";    

  if (RegisterClassEx(&style) == 0) { // failure returns 0
      throw std::runtime_error{__func__};
  }
  //hwnd = CreateFullscreenWindow(hwnd,hinstance,style.lpszClassName,caption,this);
  hwnd = CreateWindow(style.lpszClassName, caption, 
                      //WS_VISIBLE,
                      WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
                      0, 0, // dummy positions, adjusted later with ResizeWindow()
                      0, 0, // dummy width and height, adjusted later with ResizeWindow()
                      nullptr, nullptr, // hWndParent and hMenu
                      hinstance,      
                      std::addressof(event_queue_)); // smuggle into wndproc()
  if (!hwnd) {
      throw std::runtime_error{__func__};   
  }          
  //SetCursor(NULL);    
  ShowCursor(true);
  resize_window(hwnd, width_px, height_px); // remove for fullscreen                
}   


window::~window() 
{                
    if (msg.message != WM_QUIT) { // we must have thrown.. 
        // assert(std::uncaught_exceptions()); 
        PostMessage(hwnd, WM_CLOSE, 0, 0); 
        //PostQuitMessage(0);
        enter_message_loop([](auto&){}); // do nothing with messages..        
    }

}                




}} // namespaces
