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
        struct keydown { unsigned key; };
        struct mousedown {};
        struct exit_immediately {};
        struct exit_state { int exitcode; };
    }                

    class event_variant {
        using variant = vn::variant_over<none,
                                         keydown, 
                                         mousedown,                                          
                                         exit_immediately,
                                         exit_state>;
        variant value_;

    public:   
        template <typename T>
        event_variant(T const& e) : value_{e} {}  

        event_variant(event_variant const&) = default;
        event_variant(event_variant&&) = default;

        variant value() const { return value_; }

        friend bool same_type(event_variant const& lhs, event_variant const& rhs) { 
            return apply_visitor(vn::visitors::same_type{},lhs.value_,rhs.value_); 
        }

        friend bool operator==(event_variant const& lhs, event_variant const& rhs) {
            return apply_visitor(vn::visitors::weak_equality{}, lhs.value_, rhs.value_);
        }
    };


    template <typename T> // currently unused..
    void dispatch_event(T& t, event_variant e) {        
        t.emplace_back(std::move(e));
    }
    

} // namespace 

    using event = events::event_variant;

} // namespace
#endif
