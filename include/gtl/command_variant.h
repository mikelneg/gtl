/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef NBJKWWAFFEEGF_GTL_COMMAND_VARIANT_H_
#define NBJKWWAFFEEGF_GTL_COMMAND_VARIANT_H_

#include <boost/variant.hpp>

namespace gtl {

namespace commands {

    struct get_audio_adapter {
    };
    struct get_some_resource {
    }; // TODO just for testing..
    struct get_swap_chain {
    };
    struct draw {
    };

    struct resize {
        int w, h;
    };
    struct handle {
    };
}

using command_variant
    = boost::variant<commands::get_audio_adapter, commands::get_some_resource, commands::get_swap_chain, commands::draw, commands::resize, commands::handle>;

} // namespace
#endif
