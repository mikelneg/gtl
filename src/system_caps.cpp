/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/system_caps.h"
#include "gtl/pix.h"

#include <tuple>
#include <windows.h>

namespace gtl {
namespace caps {

    screen_resolution get_resolution()
    {
        HDC screen = GetDC(NULL);
        caps::screen_resolution sr{GetDeviceCaps(screen, HORZRES), GetDeviceCaps(screen, VERTRES)};
        ReleaseDC(NULL, screen);
        return sr;
    }

    pix::ppi get_ppi()
    {
        HDC screen = GetDC(NULL);
        pix::ppi ppi_{GetDeviceCaps(screen, LOGPIXELSX)};
        ReleaseDC(NULL, screen);
        return ppi_;
    }
}
} // namespace