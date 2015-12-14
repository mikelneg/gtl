#ifndef SOWIFJWBBGG_GTL_EVENT_H_
#define SOWIFJWBBGG_GTL_EVENT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::event
        
-----------------------------------------------------------------------------*/

#include <boost/variant.hpp>
#include <vn/include/boost_utilities.h>
#include <vn/include/boost_visitors.h>

namespace gtl {

    namespace event_types {
        struct none {};
        struct exitall {};
        struct keydown { unsigned key; };
        struct mousedown {};
        struct sendquit { int exitcode; };
    }
    
    struct event {
        using event_variant = vn::variant_over<
                                event_types::none,
                                event_types::keydown, 
                                event_types::mousedown, 
                                event_types::sendquit,
                                event_types::exitall>;
        event_variant value_;

        template <typename T>
        event(T const& value) : value_{value} {}
        
        friend bool same_type(event const& lhs, event const& rhs) { 
            return apply_visitor(vn::visitors::same_type{},lhs.value_,rhs.value_); 
        }
    };
        
} // namespaces
#endif
