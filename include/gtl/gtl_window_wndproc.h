/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BOAWASPOAFEFA_GTL_WINDOW_WNDPROC_H_
#define BOAWASPOAFEFA_GTL_WINDOW_WNDPROC_H_

#include <cassert>
#include <gtl/events.h>
#include <iostream>
#include <windows.h>
#include <windowsx.h>

namespace gtl {
namespace win {

    namespace detail {
        //namespace {

        //inline void set_window_user_data(HWND hwnd, LPARAM lparam) {
        //    auto tmp_ptr = reinterpret_cast<LONG_PTR>(
        //                        reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
        //    SetLastError(0);
        //    if (SetWindowLongPtr(hwnd, GWLP_USERDATA, tmp_ptr) == 0 && GetLastError() != 0)
        //    {
        //        throw std::runtime_error{__func__};
        //    }
        //}
        //template <typename T>
        //inline T& get_window_user_data(HWND hwnd) {
        //    return *reinterpret_cast<T*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
        //}
        //}

        template <typename T>
        LRESULT CALLBACK wndproc_impl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            using namespace gtl::events;

            auto handler_ref = [](HWND hwnd) -> T& { return *reinterpret_cast<T*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)); };

            auto set_user_data = [](HWND hwnd, LPARAM lparam) {
                                auto tmp_ptr = reinterpret_cast<LONG_PTR>(
                                                    reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams); 
                                SetLastError(0);
                                if (SetWindowLongPtr(hwnd, GWLP_USERDATA, tmp_ptr) == 0 && GetLastError() != 0){
                                    throw std::runtime_error{__func__};
                                } };

            // Notes:
            // Wndproc signature = LRESULT(CALLBACK*)(HWND,UINT,WPARAM,LPARAM);
            // CreateWindowEx() calls wndproc several times before it returns
            // (with [at least] messages WM_CREATE, WM_NCCREATE, WM_NCALCSIZE). When msg == WM_NCREATE
            // "lparam" contains the user-data parameter passed in CreateWindowEx().

            // consider something like: static T* t{}; with WM_NCCREATE initialization..

            switch (msg)
            {
                case WM_NCCREATE:
                {
                    set_user_data(hwnd, lparam);
                }
                break;

                //-------------------------------------------//

                case WM_KEYUP:
                {
                }
                break;

                case WM_CHAR:
                {
                    if ((HIWORD(lparam) & KF_REPEAT) < 1)
                    { // block auto-repeat
                        switch (wparam)
                        {
                            case keyboard::Backspace:
                            case keyboard::Enter:
                                handler_ref(hwnd).emplace_back(keydown{static_cast<unsigned>(wparam)});
                                break;
                            default:
                                handler_ref(hwnd).emplace_back(keydown{static_cast<unsigned>(wparam)});
                                break;
                        }
                    }
                    return 0;
                }
                break;

                case WM_KEYDOWN:
                {
                }
                break;

                case WM_SYSKEYDOWN:
                {
                }
                break;

                case WM_MOUSEWHEEL:
                {
                    POINT xy_coords{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
                    ScreenToClient(hwnd, &xy_coords);
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_wheel_scroll{GET_WHEEL_DELTA_WPARAM(wparam),
                                                                                   GET_KEYSTATE_WPARAM(wparam),
                                                                                   xy_coords.x, xy_coords.y});
                    return 0;
                }
                break;

                case WM_LBUTTONDOWN:
                {
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_lbutton_down{lparam}); // GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)});
                    //The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    //The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    return 0;
                }
                break;

                case WM_RBUTTONDOWN:
                {
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_rbutton_down{lparam}); // GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)});
                    //The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    //The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    return 0;
                }
                break;

                case WM_LBUTTONUP:
                {
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_lbutton_up{lparam}); // GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)});
                    //The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    //The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    return 0;
                }
                break;

                case WM_RBUTTONUP:
                {
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_rbutton_up{lparam}); // GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)});
                    //The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    //The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
                    return 0;
                }
                break;

                case WM_CAPTURECHANGED:
                {
                    return 0;
                }
                break;

                case WM_MOUSEMOVE:
                {
                    handler_ref(hwnd).emplace_back(gtl::events::mouse_moved{lparam}); // GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)});
                    return 0;
                }
                break;

                //case WM_NCMOUSEMOVE: break;

                //---- WM_APP custom messages ------//

                // case WM_APP + 1 : //
                // case WM_APP + X : DisableCloseButton(); return 0;
                // case WM_APP + X' : EnableCloseButton(); return 0;

                //----------------------------------//

                case WM_SETCURSOR:
                {
                    if (LOWORD(lparam) == HTCLIENT)
                    { // Hide cursor in client area..
                        SetCursor(LoadCursor(NULL, IDC_ARROW));
                        return TRUE;
                    }
                }
                break;

                case WM_NCCALCSIZE:
                {
                }
                break;

                case WM_GETMINMAXINFO:
                {
                }
                break;

                case WM_KILLFOCUS: // ReleaseCapture(); ?
                {
                }
                break;

                case WM_SETFOCUS:
                {
                    SetFocus(hwnd); // SetCapture(hwnd);
                }
                break;

                case WM_CLOSE: // issued with close button...
                {
                    DestroyWindow(hwnd);
                }
                break;

                case WM_DESTROY: // Beginning of destruction
                {
                }
                break;

                case WM_NCDESTROY:
                {
                    PostQuitMessage(0);
                }
                break;

                case WM_SIZE:
                {
                }
                    std::cout << 0;
                    break;

                case WM_MENUCHAR:
                {
                }
                    return MAKELRESULT(0, MNC_CLOSE); // disable beep on alt-enter

                default:
                {
                }
                break;
            }

            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    }
}
} // namespaces
#endif