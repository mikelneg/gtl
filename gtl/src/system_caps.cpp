#include "../include/system_caps.h"
#include "../include/pix.h"

#include <windows.h>
#include <tuple>

namespace gtl {
namespace caps {    

    screen_resolution get_resolution()
    {        
        HDC screen = GetDC(NULL);
        caps::screen_resolution sr{GetDeviceCaps(screen,HORZRES),GetDeviceCaps(screen,VERTRES)};
        ReleaseDC(NULL,screen);
        return sr;
    }
    
    pix::ppi get_ppi()
    {        
        HDC screen = GetDC(NULL);
        pix::ppi ppi_{GetDeviceCaps(screen,LOGPIXELSX)};                            
        ReleaseDC(NULL,screen);
        return ppi_;
    }
}
} // namespace