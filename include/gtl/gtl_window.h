/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YWIWVNWMASDF_GTL_WINDOW_H_
#define YWIWVNWMASDF_GTL_WINDOW_H_

#include <gtl/events.h>
#include <vector>
#include <windows.h>

namespace gtl {
namespace win {

    class window {
        HWND hwnd;
        MSG msg;
        unsigned width_px, height_px;
        std::vector<gtl::event> event_queue_;

    public:
        window(HINSTANCE hinst, unsigned w_px, unsigned h_px, const char* caption);
        ~window();

        window(window&&) = delete;
        window& operator=(window&&) = delete;

        template <typename F>
        void enter_message_loop(F func)
        {
            while (true)
            {
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                { // we use NULL instead of hwnd so that we see WM_QUIT
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                if (msg.message == WM_QUIT)
                {
                    return;
                }
                func(event_queue_);
                event_queue_.clear();
            }
        }

        friend auto width(window& w) noexcept
        {
            return w.width_px;
        }
        friend auto height(window& w) noexcept
        {
            return w.height_px;
        }
        friend auto get_hwnd(window& w) noexcept
        {
            return w.hwnd;
        }
    };
}
} // namespaces
#endif
