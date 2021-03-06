/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef SOWIFJWBBGG_GTL_EVENTS_H_
#define SOWIFJWBBGG_GTL_EVENTS_H_

#include <string>

#include <boost/variant.hpp>
#include <vn/boost_variant_utilities.h>
#include <vn/boost_visitors.h>

namespace gtl {
namespace events {

    inline namespace event_types {
        struct done {
        };

        struct dump_contents {
            std::string id;
        };

        struct focus_entity {
            std::string id;
        };

        struct exit_all {
        };

        //
        struct dpad_pressed {
            float x, y;
        };

        struct keep {
        };

        struct revert {
        };

        struct none {
        };

        struct keydown {
            unsigned key;
        };

        struct keyup {
            unsigned key;
        };

        struct mouse_lbutton_down {
            int x, y;
        };

        struct mouse_rbutton_down {
            int x, y;
        };

        struct mouse_lbutton_up {
            int x, y;
        };

        struct mouse_rbutton_up {
            int x, y;
        };

        struct mouse_wheel_scroll {
            int wheel_delta;
            int key_state;
            int x, y;
        };

        struct mouse_click {
            std::string id;
            int x, y;
        };

        struct mouse_moved {
            int x, y;
        }; // TODO revisit: currently int64_t because windows uses hi and low bits to set both x and y coord..

        struct mousedown {
        };

        struct exit_immediately {
        };

        struct exit_state {
            int exitcode;
        };

        struct resize_swapchain {
            int new_width, new_height;
        };

        using mouse_event = boost::variant<mouse_lbutton_down, mouse_rbutton_down, mouse_lbutton_up, mouse_rbutton_up, mouse_wheel_scroll, mouse_click,
                                           mouse_moved, mousedown>;
    }

    // TODO this variant needs to be broken into several different variantes (there is a limit of 16 types by default)

    using event_variant_base_ = boost::variant<done, dump_contents, focus_entity, exit_all, none, keep, revert, dpad_pressed, keydown, keyup, mouse_event,
                                               exit_immediately, exit_state, resize_swapchain>;

    // class event_variant : public event_variant_base_ {
    //
    // public:
    //    template <typename ...Args>
    //    event_variant(Args&&...args) noexcept(noexcept(event_variant_base_(std::forward<Args>(args)...)))
    //        : event_variant_base_(std::forward<Args>(args)...) {}
    //
    //    event_variant(event_variant const&) = default;
    //    //event_variant(event_variant&&) = default;
    //    //event_variant& operator=(event_variant&&) = default;
    //    event_variant& operator=(event_variant const&) = default;
    //
    //    friend bool same_type(event_variant const& lhs, event_variant const& rhs) {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::same_type{},lhs,rhs);
    //    }
    //
    //    friend bool operator==(event_variant const& lhs, event_variant const& rhs) {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::weak_equality{},lhs,rhs);
    //    }
    //
    //    template <typename T>
    //    friend bool has_variant_type(event_variant const& e) {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::has_variant_type<T>{},e);
    //    }
    //};

    using event_variant = event_variant_base_;

    // class event_variant { // HACK replace this clunky .value() interface..
    //
    //    using variant = event_variant_base_;
    //    variant value_;
    //
    // public:
    //    template <typename T>
    //    event_variant(T const& e) : value_{e}
    //    {
    //    }
    //
    //    event_variant(event_variant const&) = default;
    //    event_variant(event_variant&&) = default;
    //
    //    variant value() const
    //    {
    //        return value_;
    //    }
    //
    //    friend bool same_type(event_variant const& lhs, event_variant const& rhs)
    //    {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::same_type{}, lhs.value_, rhs.value_);
    //    }
    //
    //    friend bool operator==(event_variant const& lhs, event_variant const& rhs)
    //    {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::weak_equality{}, lhs.value_, rhs.value_);
    //    }
    //
    //    template <typename T>
    //    friend bool has_variant_type(event_variant const& e)
    //    {
    //        using boost::apply_visitor;
    //        return apply_visitor(vn::visitors::has_variant_type<T>{}, e.value());
    //    }
    //
    //    // operator variant&() { return value_; }
    //};

    // template <typename T> // currently unused..
    // void dispatch_event(T& t, event_variant e)
    //{
    //    t.emplace_back(std::move(e));
    //}

} // namespace

using event = events::event_variant;

} // namespace
#endif
