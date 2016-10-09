/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UWUHFWZZSFF_GTL_GUI_H_
#define UWUHFWZZSFF_GTL_GUI_H_

/*-------------------------------------------------------------

not used -- keeping for reference

---------------------------------------------------------------*/

#include <exception>

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>

#include <tuple>

#include <boost/variant.hpp>
#include <vn/boost_variant_utilities.h>
#include <vn/boost_visitors.h>

//#include <boost/coroutine/asymmetric_coroutine.hpp>
//#include <boost/coroutine/symmetric_coroutine.hpp>

#include <gtl/events.h>
#include <gtl/gtl_render_properties.h>

namespace gtl {
namespace gui {
    /*
        class ui_variant;
        // using ui_variant_base_type_ = boost::variant<empty, layer, textbox, button, std::unique_ptr<custom>>;

        class layer;
        class textbox;
        class button;
        class behavior;
        class custom;
        //...
        class empty {};

        using id = std::string;

        using event_stream = boost::coroutines::asymmetric_coroutine< gtl::event const& >;
        using event_pull = event_stream::pull_type;
        using event_push = event_stream::push_type;

        using gui_listener_params = std::tuple<layer&, gtl::event const&, event_push&>;
        using gui_listener = boost::coroutines::asymmetric_coroutine< gui_listener_params >;
        using gui_listener_push = gui_listener::push_type;
        using gui_listener_pull = gui_listener::pull_type;

        using gui_channel_params = std::tuple<layer&, event_pull&, event_push&>; // reference to parent, reference to
       msg stream

        using gui_channel = boost::coroutines::asymmetric_coroutine< gui_channel_params >;
        using gui_pull = gui_channel::pull_type;
        using gui_push = gui_channel::push_type;


        struct listener_registry {
            std::vector< gui_listener_push > listeners_;

            template <typename T>
            void emplace_back(T&& t) { listeners_.emplace_back(std::forward<T>(t)); }

            template <typename T>
            friend auto begin(T&& t) { using std::begin; return begin(t.listeners_); }

            template <typename T>
            friend auto end(T&& t) { using std::begin; return end(t.listeners_); }
        };

        //
        class layer {
            std::unordered_map<id, ui_variant> children_;
        public:
            layer() = default;

            layer& add(id, ui_variant&& g) &;
            layer&& add(id, ui_variant&& g) &&;

            layer& register_listener(id const& child_, listener_registry&) &;
            layer&& register_listener(id const& child_, listener_registry& reg_) && { register_listener(child_,reg_);
       return std::move(*this); }

            ui_variant& get_child(id const&);
        };

        //
        class behavior {
            std::function<void(gui_pull&)> func_;
        public:
            template <typename F>
            behavior(F&& f) noexcept(noexcept(func_(std::forward<F>(f))))
                : func_(std::forward<F>(f)) {}

            inline void trigger(gui_pull& f) const { func_(f); }
        };


        class textbox {
            std::string text_;
        public:
            textbox(std::string start_text_) : text_{std::move(start_text_)} {}

            void set_text(std::string new_text_) { using std::swap; swap(text_,new_text_); }
            std::string const& get_text() const { return text_; }

            void set_focus(gui_listener_pull&);
            void listen(gui_listener_pull&);

            //void set_focus(co::pull_type&);
            //void set_focus(msg_co::pull_type&);
            //void observe(msg_observer::pull_type&);
        };

        //

        class button {
            std::function<void(gui_listener_pull&)> func_;
        public:
            template <typename F>
            button(F&& f) //noexcept(noexcept(func_(std::forward<F>(f))))
            : func_(std::forward<F>(f)) {}

            void click(gui_listener_pull&) const;
        };

        //

        class custom {
        public:
            // TODO revisit the interface for this..
            virtual void set_focus(gui_pull&) = 0;
            virtual ~custom() {}
        };

        // class derived_ui : public custom {  // example..
        //      std::string text;
        //      derived_ui(std::string s) : text{s} {}
        //      void set_focus(gui_pull& focus_) override {}
        //  };
        //

        using ui_variant_base_type_ = boost::variant<empty, layer, textbox, button, std::unique_ptr<custom>>;

        class ui_variant : public ui_variant_base_type_ {
        public:
            template <typename ...Args>
            ui_variant(Args&&...args) noexcept(noexcept(ui_variant_base_type_(std::forward<Args>(args)...)))
                    : ui_variant_base_type_(std::forward<Args>(args)...) {}
        };


        // generic interfacing

        template <typename T, typename R>
        void listen(T&,R&) { throw std::runtime_error{"gtl_gui:: generic listen(T&,R&) should not be called.."}; }

        //template <typename T, typename R>
        //void set_focus(T&,R&) { throw std::runtime_error{"gtl_gui:: generic set_focus(T&,R&) should not be called.."};
       }

        inline void listen(ui_variant& u, gui_listener_pull& p) {
            auto vis = vn::make_lambda_visitor(
                                 [](auto&) {},
                                 [&](textbox& v) { v.listen(p); });
            boost::apply_visitor(vis,u);
        }

        inline void set_focus(ui_variant& u, gui_listener_pull& p) {
            auto vis = vn::make_lambda_visitor(
                                 [](auto&) {},
                                 [&](textbox& v) { v.set_focus(p); });
            boost::apply_visitor(vis,u);
        }

        inline ui_variant& get_child(layer& parent, id name) { return parent.get_child(name); }


            inline layer& parent(gui_channel_params& p) { return std::get<0>(p); }
            inline event_pull& msg_source(gui_channel_params& p) { return std::get<1>(p); }
            inline gtl::event const& msg(gui_channel_params& p) { return msg_source(p).get(); }
            inline event_push& controller(gui_channel_params& p) { return std::get<2>(p); }

            inline layer& parent(gui_listener_params& p) { return std::get<0>(p); }
            inline gtl::event const& msg(gui_listener_params& p) { return std::get<1>(p); }
            inline event_push& controller(gui_listener_params& p) { return std::get<2>(p); }

        //

        */
}
} // namespaces

#endif