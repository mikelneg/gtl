#ifndef SOWIFJWBBGG_GTL_EVENTS_H_
#define SOWIFJWBBGG_GTL_EVENTS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::event
        
-----------------------------------------------------------------------------*/

#include <boost/variant.hpp>
#include <vn/include/boost_utilities.h>
#include <vn/include/boost_visitors.h>

namespace gtl {
namespace events {

    inline namespace event_types {
        struct none {};
        struct exitall {};
        struct keydown { unsigned key; };
        struct mousedown {};
        struct sendquit { int exitcode; };
    }                

    struct event_variant {
        using variant = vn::variant_over<none,
                                         keydown, 
                                         mousedown, 
                                         sendquit,
                                         exitall>;
        variant value_;
   
        template <typename T>
        event_variant(T const& e) : value_{e} {}  

        event_variant(event_variant const&) = default;
        event_variant(event_variant&&) = default;

        friend bool same_type(event_variant const& lhs, event_variant const& rhs) { 
            return apply_visitor(vn::visitors::same_type{},lhs.value_,rhs.value_); 
        }
    };


    template <typename T>
    void dispatch_event(T& t, event_variant e) {
        // currently only using std::vector<event_variant>
        t.emplace_back(std::move(e));
    }
    

}

using event = events::event_variant;

} // namespaces
#endif
