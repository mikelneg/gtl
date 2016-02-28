#ifndef UWUHFWZZSFF_GTL_GUI_H_
#define UWUHFWZZSFF_GTL_GUI_H_

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
            
    //using listener = boost::coroutines::asymmetric_coroutine<std::pair<int&, gtl::gui::layer&>>;
    //using observer = boost::coroutines::asymmetric_coroutine<std::pair<gtl::gui::ui_variant&, std::string>>::push_type;
    //using msg_registry = std::unordered_multimap<std::string,observer>;
       
    using event_stream = boost::coroutines::asymmetric_coroutine< gtl::event& >;
    using event_pull = event_stream::pull_type;    
    using event_push = event_stream::push_type;
        
    using gui_channel_params = std::pair<layer&, event_pull&>; // reference to parent, reference to msg stream

    using gui_channel = boost::coroutines::asymmetric_coroutine< gui_channel_params >;
    using gui_pull = gui_channel::pull_type;
    using gui_push = gui_channel::push_type;
    
    //template <typename T>
    //struct msg_reg {
    //    std::vector<msg_observer::push_type> reg;
    //
    //    template <typename F>
    //    void attach(F&& f) {
    //        reg.emplace_back(std::forward<F>(f));
    //    }
    //
    //    friend auto begin(msg_reg& m) { return m.reg.begin(); }
    //    friend auto end(msg_reg& m) { return m.reg.end(); }
    //};
    
    // some generic interface:
    //    listen(p,q);
    //
    // using listener_registry = std::vector< event_push >;    

    inline layer& get_parent_from(gui_channel_params& p) { return p.first; }
    inline layer& get_parent_from(gui_pull& p) { return get_parent_from(p.get()); }    
    
    struct listener_registry {
        std::vector< gui_push > listeners_;

        template <typename T>
        void emplace_back(T&& t) { listeners_.emplace_back(std::forward<T>(t)); }

        template <typename T>
        friend auto begin(T&& t) { using std::begin; return begin(t.listeners_); }
        
        template <typename T>
        friend auto end(T&& t) { using std::begin; return end(t.listeners_); }        
    };

    // generic interfacing

    template <typename T, typename R>
    void listen(T&,R&) { throw std::runtime_error{"gtl_gui:: generic listen(T&,R&) should not be called.."}; } 
        
    inline void listen(ui_variant& u, gui_pull& p) { boost::apply_visitor([&](auto& v){ listen(v,p); },u); }

    //
    class layer {
        std::unordered_map<id, ui_variant> children_;                
    public:         
        layer() = default;
    
        layer& add(id, ui_variant&& g) &;
        layer&& add(id, ui_variant&& g) &&;
                                   
        layer& register_listener(id const& child_, listener_registry&) &;                            
        layer&& register_listener(id const& child_, listener_registry& reg_) && { register_listener(child_,reg_); return std::move(*this); } 
                    
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
        
        void give_focus(gui_channel_params, event_push&);                
        void give_observer(gui_channel_params);

        //void set_focus(co::pull_type&);        
        //void set_focus(msg_co::pull_type&);
        //void observe(msg_observer::pull_type&);  

        template <typename M>
        friend void listen(textbox& t, M& m) { t.listen(m); }
    };

    //

    class button {
        std::function<void(gui_pull&)> func_;      
    public:
        template <typename F> 
        button(F&& f)   noexcept(noexcept(func_(std::forward<F>(f))))
            : func_(std::forward<F>(f)) {}    
                
        inline void click(gui_pull& f) const { func_(f); }            
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

#endif 