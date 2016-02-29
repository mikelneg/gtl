#include "../include/gtl_gui.h"

#include <exception>

#include <iostream>
#include <functional>
#include <unordered_map>
#include <utility>
#include <memory>
#include <chrono>

#include <boost/variant.hpp>
#include <vn/include/boost_utilities.h>
#include <vn/include/boost_visitors.h>

#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <gtl/include/events.h>

namespace gtl { 
namespace gui {  
    
    //namespace {
    //    static inline layer& parent(gui_channel_params& p) { return std::get<0>(p); }
    //    static inline event_pull& msg_source(gui_channel_params& p) { return std::get<1>(p); }
    //    static inline gtl::event const& msg(gui_channel_params& p) { return msg_source(p).get(); }
    //    static inline event_push& controller(gui_channel_params& p) { return std::get<2>(p); }        
    //}

    void custom::set_focus(gui_pull&) {} // default implementation for pure virtual..    

    //ui_variant& get_child(id const& id_, ui_variant& v) {  
    //    auto vis = vn::make_lambda_visitor<ui_variant&>(
    //                    [&](auto&)->decltype(auto) { throw std::runtime_error{"gui::get_child -- non-layer variant does not have children"}; return v; },
    //                    [&](layer& layer)->decltype(auto){ return layer.get_child(id_); });                    
    //    return boost::apply_visitor(vis, v);    
    //}
    //
    //ui_variant& get_child(id const& id_, layer& parent) {
    //    return parent.get_child(id_);
    //}
    //
    //void set_focus(ui_variant& target_, gui_pull& focus_) {
    //    auto vis = vn::make_lambda_visitor([](auto&){},  // TODO assert/throw in catch-all, add additional cases..
    //                                       [&](textbox& b){ b.give_focus(focus_); });
    //    boost::apply_visitor(vis,target_);
    //}        

    layer& layer::add(id i, ui_variant&& o) & { 
        children_.emplace(std::move(i), std::move(o)); 
        return *this; 
    }  

    layer&& layer::add(id i, ui_variant&& o) && { 
        children_.emplace(std::move(i), std::move(o)); 
        return std::move(*this); 
    }

    ui_variant& layer::get_child(id const& id_) { 
        if (children_.count(id_) == 0) {
            throw std::runtime_error{"gui::layer::get_child() - child not found in layer."};
        }
        return children_[id_]; 
    }  // TODO revisit..       
    
    layer& layer::register_listener(id const& name, listener_registry& msg) & {

        msg.emplace_back([name=name](gui_listener_pull& p){ // we emplace a function that calls into our listener with our pull_type coroutine
            listen(parent(p.get()).get_child(name), p); 
            //boost::apply_visitor([&](auto& v) { listen(v,p); }, get_parent_from(p).get_child(name)); 
        });

        return *this;    
    }                
      
    void textbox::set_focus(gui_listener_pull& p) {        
        using gtl::events::has_variant_type;
        std::cout << "textbox taking focus..\n";               
        while (!has_variant_type<gtl::events::exit_all>(msg(p.get()))) {
            if (has_variant_type<gtl::events::none>(msg(p.get()))) {
                //yield.get().second(msg::none{});
                if (controller(p.get())) {
                    controller(p.get())(gtl::events::none{});
                }
            } else {
                std::cout << "[focus] textbox ..\n"; 
            }
            p();
        }
        std::cout << "textbox releasing msg focus..\n";
    }
    
    void textbox::listen(gui_listener_pull& p) {
        using gtl::events::has_variant_type;
        std::cout << "textbox observing..\n";        
        while (!has_variant_type<gtl::events::exit_all>(msg(p.get()))) {
            if (has_variant_type<gtl::events::none>(msg(p.get()))) {
                std::cout << "[textbox noticed the <none> message!]\n";
            } else
            if (has_variant_type<gtl::events::mouse_click>(msg(p.get()))) {        
                try {
                    std::cout << "[textbox noticed the <click> message for " << 
                        (boost::get<gtl::events::mouse_click>(msg(p.get())).id) << "!]\n";
                }
                catch(...) {
                    std::cout << "textbox caught an exception....\n";
                }
            }        
            p();
        }
        std::cout << "textbox done observing..\n";
    }

    void button::click(gui_listener_pull& p) const { func_(p); }

}} // namespaces

