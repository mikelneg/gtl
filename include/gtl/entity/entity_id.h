/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef WUWOOBOAFEF_GTL_ENTITY_ENTITY_ID_H_
#define WUWOOBOAFEF_GTL_ENTITY_ENTITY_ID_H_

/*-------------------------------------------------------------

    gtl::common::entity_id

    -   The id type used to bridge the various components of the
        engine

---------------------------------------------------------------*/

#include <cstdint>

namespace gtl {
namespace entity {

    using id_type = uint16_t;
    using id = id_type;
}
} // namespaces
#endif
