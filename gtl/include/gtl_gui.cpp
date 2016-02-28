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
    
    void custom::set_focus(gui_pull&) {} // default implementation for pure virtual..    

    ui_variant& get_child(id const& id_, ui_variant& v) {  
        auto vis = vn::make_lambda_visitor<ui_variant&>(
                        [&](auto&)->decltype(auto) { throw std::runtime_error{"gui::get_child -- non-layer variant does not have children"}; return v; },
                        [&](layer& layer)->decltype(auto){ return layer.get_child(id_); });                    
        return boost::apply_visitor(vis, v);    
    }

    ui_variant& get_child(id const& id_, layer& parent) {
        return parent.get_child(id_);
    }
    
    void set_focus(ui_variant& target_, gui_pull& focus_) {
        auto vis = vn::make_lambda_visitor([](auto&){},  // TODO assert/throw in catch-all, add additional cases..
                                           [&](textbox& b){ b.set_focus(focus_); });
        boost::apply_visitor(vis,target_);
    }        

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

    template <typename M>
    layer& layer::register_listener(id const& name, M& msg) & {

        msg.emplace_back([name=name](gui_pull& p){ // we emplace a function that calls into our listener with our pull_type coroutine
            listen(get_parent_from(p).get_child(name), p); 
            //boost::apply_visitor([&](auto& v) { listen(v,p); }, get_parent_from(p).get_child(name)); 
        });

        return *this;    
    }                
      
    void textbox::give_focus(gui_channel_params params_, event_push& controller_) {        
        using gtl::events::has_variant_type;
        std::cout << "textbox taking focus..\n";        
        event_pull& msg_stream_ = params_.second;
        while (!has_variant_type<gtl::events::done>(msg_stream_.get())) {
            if (has_variant_type<gtl::events::none>(msg_stream_.get())) {
                //yield.get().second(msg::none{});
                if (controller_) {
                    controller_(gtl::events::none{});
                }
            } else {
                std::cout << "[focus] textbox ..\n"; 
            }
            msg_stream_();
        }
        std::cout << "textbox releasing msg focus..\n";
    }
    
    void textbox::give_observer(gui_channel_params params_) {
        using gtl::events::has_variant_type;
        std::cout << "textbox observing..\n";
        event_pull& msg_stream_ = params_.second;
        while (!has_variant_type<gtl::events::done>(msg_stream_.get())) {
            if (has_variant_type<gtl::events::none>(msg_stream_.get())) {
                std::cout << "[textbox noticed the <none> message!]\n";
            } else
            if (has_variant_type<gtl::events::mouse_click>(msg_stream_.get())) {        
                try {
                    std::cout << "[textbox noticed the <click> message for " << 
                        boost::get<gtl::events::mouse_click>(msg_stream_.get()).id << "!]\n";
                }
                catch(...) {
                    std::cout << "textbox caught shit..\n";
                }
            }        
            msg_stream_();
        }
        std::cout << "textbox done observing..\n";
    }





}} // namespaces



struct dummy {
    std::string data;    
    void update(std::string const& t) { data = t; }    
    void update(msg::msg_type const& m) { 
        auto vis = vn::make_lambda_visitor([](auto const&){ std::cout << "msg_type wrong, dummy not updated..\n"; },
                                           [this](msg::none const&){  std::cout << "dummy updated!\n"; data = "none!!"; });
        boost::apply_visitor(vis,m);
    }
};


template <typename T>
struct msg_reg {
    std::vector<msg_observer::push_type> reg;

    template <typename F>
    void attach(F&& f) {
        reg.emplace_back(std::forward<F>(f));
    }

    friend auto begin(msg_reg& m) { return m.reg.begin(); }
    friend auto end(msg_reg& m) { return m.reg.end(); }
};


int main() {
    using namespace gtl::gui;

    std::unordered_multimap<std::string,observer> reg;

    msg_reg<msg::none> msg_x_reg;
    //msg_registry reg;

    ui_variant g{ layer{}.add("main-layer", 
                              layer{}.add("input", textbox{"type some junk here"})
                                     .add("othertext",textbox{"default junk"}).attach_msg_observer("othertext",msg_x_reg)
                                     .add("brothertext",textbox{"default junk also"}).attach_msg_observer("brothertext",msg_x_reg)                          
                                     .add("plink", std::unique_ptr<poly_ui>{std::make_unique<derived_ui>("silly dolphin")})
                                     .add("input-clear", button{[](ui_variant& parent, msg_co::pull_type&) {  
                                                        ui_variant& c = get_child(parent,"input"); 
                                                        set_value(c,std::string{"<cleared>"});  
                                                        std::cout << "clicked!\n";                                                      
                                                    }})                                     
                                ) };

    dummy d{"dummy"};

    std::function<void(std::string&)> dummy_controller{[&](std::string& up){ 
                                                            std::cout << "dummy_object updated! was = [" << d.data << "]";
                                                            d.update(up); std::cout << ", now = [" << d.data << "]\n"; }};

    std::function<void(msg::msg_type const&)> dummy_msg_controller{[&](msg::msg_type const& m){                                                            
                                                            std::cout << "dummy object about to update! currently = [" << d.data << "]\n";                                                            
                                                            d.update(m); 
                                                            std::cout << "now = [" << d.data << "]\n"; }};


//    co::push_type focus{[&](auto& yield){
//        while (yield.get().first != "done") {
//            if (yield.get().first == "plinkit") {
//                ui_variant& p = get_child(g,"main-layer");
//                ui_variant& b = get_child(p,yield().get().first);
//
//                click(p, b, yield()); 
//
//            } else
//            if (yield.get().first == "setfocus") {
//                ui_variant& p = get_child(g,"main-layer");
//                ui_variant& b = get_child(p,yield().get().first);
//
//                set_focus(b, yield()); 
//
//            } else
//            if (yield.get().first == "click") {
//
//                ui_variant& p = get_child(g,"main-layer");
//                ui_variant& b = get_child(p,yield().get().first);
//
//                click(p, b, yield());
//            } else
//            if (yield.get().first == "show") {
//
//                ui_variant& p = get_child(g,"main-layer");
//                ui_variant& b = get_child(p,yield().get().first);
//
//                std::cout << get_value(b) << "\n";
//            }        
//
//            yield(); 
//        } 
//    }};    

    msg_co::push_type msg_focus{[&](msg_co::pull_type& yield){
        using msg::is_same;
        while (!is_same<msg::exit_all>(yield.get().first)) {
            if (is_same<msg::mouse_click>(yield.get().first)) {
                
                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,boost::get<msg::mouse_click&>(yield.get().first).id);
                std::cout << "mouse click..\n";
                click(p, b, yield()); 

            } else
            if (is_same<msg::entity_focused>(yield.get().first)) {
                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,boost::get<msg::entity_focused&>(yield.get().first).id);

                set_focus(b, yield()); 

            } else
            if (is_same<msg::dump_contents>(yield.get().first)) {

                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,boost::get<msg::dump_contents&>(yield.get().first).id);

                std::cout << get_value(b) << "\n";
            }                    
            yield(); 
        } 
    }};    


    std::vector<std::string> user_input{
        "plinkit",
        "plink",
        "fuurt",
        "setfocus", 
        "input",
        "wooakdfajsf",        
        "done",
        "show","othertext",
        "show","brothertext",        
        "settext","bloop",
        "show","othertext",
        "show","brothertext",
        "show","input",
        "click",
        "input-clear",
        "show","input",
        "setfocus", "input",
        "one more time!!",
        "settext",
        "done",
        "show","othertext",
        "show","brothertext",
        "done",
        "done"}; 

    std::vector<std::string> system_events{
        "foo1","foo2","foo3"
    }; 

    std::vector<msg::msg_type> msg_input{
        msg::entity_focused{"input"},
        msg::none{},
        msg::done{},
        msg::dump_contents{"input"},
        msg::dump_contents{"othertext"},
        msg::dump_contents{"brothertext"},
        msg::none{}, // should trigger the listener textbox        
        msg::mouse_click{"input-clear"},
        msg::dump_contents{"input"},        
        msg::done{},
        msg::exit_all{},
        msg::done{}
    };

    using msg::is_same;

    for (auto&& i : msg_input) {
        if (msg_focus) {                     
            msg_focus({i, dummy_msg_controller});
        }

        if (msg::is_same<msg::none>(i) || msg::is_same<msg::done>(i) || msg::is_same<msg::exit_all>(i) || msg::is_same<msg::mouse_click>(i)) {
            for (auto&& e : msg_x_reg) {
                if (e) {
                    e({boost::get<gtl::gui::layer&>(get_child(g,"main-layer")),i});
                }
            }
        }
    }

    //int x{1};
    //for (int i = 0; i < 10000; ++i) {
    //    boost::get<layer>(g).add_listener([](auto& yield){
    //                                while (yield.get().first != 0) { 
    //                                    //std::cout << "Listener got " << yield.get().first << "\n";
    //                                    yield.get().first++;
    //                                    yield();                                        
    //                                }
    //                            });
    //}
    
    //for (auto&& i : user_input) {        
    //    if (focus) {
    //        focus({i, dummy_controller});
    //    }
    //        auto it = reg.equal_range(i);                    // string, ...                    
    //        for (; it.first != it.second; ++it.first) {      //  
    //            //std::cout << "dumping into " << (it.first)->first << "\n";
    //            auto& r = (it.first)->second;
    //            if (r)
    //                r({get_child(g,"main-layer"),i});
//
//    //                // listener_coroutine
//    //        
//    //    }
    //}


}

/* // testing stuff

    auto time_now = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        boost::get<layer>(g).dispatch(x);
    }

    int y{0};
    boost::get<layer>(g).dispatch(y);

    auto time_later = std::chrono::high_resolution_clock::now();

    std::cout << "final value for x == " << x << "\n";
    std::cout << "compute time == " << std::chrono::duration_cast<std::chrono::milliseconds>(time_later - time_now).count() << "\n";


    std::cout << "done -- dummy == " << dummy_object << "\n";    

*/


/*

int main() { // testing version, gets about 2100 messages dispatched to 10000 listeners per millisecond
    using namespace gtl::gui;

    ui_variant g{ layer{}.add("main-layer", 
                              layer{}.add("input", textbox{"type some junk here"})                          
                                     .add("plink", std::unique_ptr<poly_ui>{std::make_unique<derived_ui>("silly dolphin")})
                                     .add("input-clear", button{[](ui_variant& parent, co::pull_type&) {  
                                                        ui_variant& c = get_child(parent,"input"); 
                                                        set_value(c,std::string{"<cleared>"});  
                                                        std::cout << "clicked!\n";                                                      
                                                    }})                                     
                                ) };

    std::string dummy_object{"bleet"};

    auto dummy_interface = [&](auto& up) { std::cout << "dummy_object updated! was = [" << dummy_object << "]";
                                           dummy_object = up; 
                                           std::cout << ", now = [" << dummy_object << "]\n"; };

    co::push_type focus{[&](auto& yield){
        while (yield.get().first != "done") {
            if (yield.get().first == "plinkit") {
                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,yield().get().first);

                click(p, b, yield()); 

            } else
            if (yield.get().first == "setfocus") {
                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,yield().get().first);

                set_focus(b, yield()); 

            } else
            if (yield.get().first == "click") {

                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,yield().get().first);

                click(p, b, yield());
            } else
            if (yield.get().first == "show") {

                ui_variant& p = get_child(g,"main-layer");
                ui_variant& b = get_child(p,yield().get().first);

                std::cout << get_value(b) << "\n";
            }        

            yield(); 
        } 
    }};    

    std::vector<std::string> user_input{
        "plinkit",
        "plink",
        "fuurt",
        "setfocus", 
        "input",
        "wooakdfajsf",
        "done",
        "show",
        "input",
        "click",
        "input-clear",
        "show",
        "input",
        "setfocus", "input",
        "one more time!!",
        "done",
        "done"}; 

    std::vector<std::string> system_events{
        "foo1","foo2","foo3"
    }; 

    int x{1};

    for (int i = 0; i < 10000; ++i) {
        boost::get<layer>(g).add_listener([](auto& yield){
                                    while (yield.get().first != 0) { 
                                        //std::cout << "Listener got " << yield.get().first << "\n";
                                        yield.get().first++;
                                        yield();                                        
                                    }
                                });
    }

    for (auto&& i : user_input) {        
        if (focus)
            focus({i, dummy_interface});
    }

    auto time_now = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        boost::get<layer>(g).dispatch(x);
    }

    int y{0};
    boost::get<layer>(g).dispatch(y);

    auto time_later = std::chrono::high_resolution_clock::now();

    std::cout << "final value for x == " << x << "\n";
    std::cout << "compute time == " << std::chrono::duration_cast<std::chrono::milliseconds>(time_later - time_now).count() << "\n";


    std::cout << "done -- dummy == " << dummy_object << "\n";    


}*/
