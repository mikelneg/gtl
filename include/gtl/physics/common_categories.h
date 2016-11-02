/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef HBWOAOSIJFAEFR_GTL_PHYSICS_COMMON_CATEGORIES_H_
#define HBWOAOSIJFAEFR_GTL_PHYSICS_COMMON_CATEGORIES_H_

#include <cstdint>

namespace gtl {
namespace physics {

    // common collision categories for Box2D
    // underlying_type is uint16_t to match Box2D's implementation

    struct collision_category {
        enum category : uint16_t {

            BOUNDARY = 1,
            ENTITY = 2,
            SENSORY = 4

        };
    };
}
} // namespace
#endif
