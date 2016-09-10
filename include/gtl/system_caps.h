/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BGHJSVBSW_GTL_SYSTEM_CAPS_H_
#define BGHJSVBSW_GTL_SYSTEM_CAPS_H_

#include "literals.h"
#include "pix.h"

#include <tuple>
#include <utility>

namespace gtl {
namespace caps {

    inline namespace cap_types_v0 {
        using screen_resolution = std::pair<int, int>;
    }

    screen_resolution get_resolution();
    pix::ppi get_ppi();

    inline pix::ppi get_base_ppi() noexcept
    {
        return pix::ppi{96};
    }
}
} // namespace
#endif
