#ifndef YWAVBVZSFWFW_GTL_EVENT_LISTENER_H_
#define YWAVBVZSFWFW_GTL_EVENT_LISTENER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl
    
    template <typename T, typename M>
    class event_listener; 
        + CRTP base class used for receiving events from a gtl::event_generator<M>               

-----------------------------------------------------------------------------*/

#include <gtl/event_generator.h>

namespace gtl {    

template <typename T, typename M>
class event_listener {
    
    using event_generator = event_generator<M>;
    using event_generator_ref = std::reference_wrapper<event_generator>;      

    std::vector<event_generator_ref> event_generators_;
    bool any_attached_{false};    

public:

    event_listener() = default;

    event_listener(event_listener&& o) 
        : event_generators_{o.detach_event_listeners()}
    {
        attach_listeners();
    }
    
    event_listener(event_generator& m) 
    {
        attach_listener(m);        
    }  
        
    ~event_listener() {
        if (any_attached_)
            detach_listeners();
    }

    bool attached() const noexcept { return any_attached_; }    
    
    std::vector<event_generator_ref> detach_listeners();        
    void attach_listeners(std::vector<event_generator_ref>);
    
    void attach_listener(event_generator&);            
    void attach_listeners();    
};

template <typename T, typename M>    
auto event_listener<T,M>::detach_listeners() -> std::vector<event_generator_ref> 
{
    for (auto&& e : event_generators_) {
        e.get().remove_listener(static_cast<T*>(this));
    }
    
    any_attached_ = false;

    std::vector<event_generator_ref> tmp; 
    tmp.swap(event_generators_);
    return tmp;
}

template <typename T, typename M>
void event_listener<T,M>::attach_listeners() {
    any_attached_ = true;
    for (auto&& e : event_generators_) {
        e.get().add_listener(static_cast<T*>(this), [](void* o, M x){ static_cast<T*>(o)->listen(x); });
    }        
}

template <typename T, typename M>
void event_listener<T,M>::attach_listener(event_generator& m) {
    any_attached_ = true;
    m.add_listener(static_cast<T*>(this), [](void* o, M x){  static_cast<T*>(o)->listen(x); });
    event_generators_.emplace_back(m);                
}


template <typename T, typename M>
void event_listener<T,M>::attach_listeners(std::vector<event_generator_ref> v) {
    for (auto&& e : v) {
        this->attach_listener(e);
    }
}

} // namespace
#endif
