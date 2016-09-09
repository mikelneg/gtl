#ifndef UWIFAOBSAFEFE_GTL_EVENT_GENERATOR_H_
#define UWIFAOBSAFEFE_GTL_EVENT_GENERATOR_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl
    
    template <typename M>
    class event_generator; 
        + registers listeners, sends them messages..

-----------------------------------------------------------------------------*/

#include <utility>
#include <vector>

namespace gtl {

template <typename M>
class event_generator {

    std::vector<std::pair<void*, void (*)(void*, M)> > listeners_;

public:
    event_generator() = default;

    void add_listener(void* obj, void (*f)(void*, M))
    {
        auto it = find_if(begin(listeners_), end(listeners_), [&](auto& x) { return x.first == obj; });
        if (it == end(listeners_)) {
            listeners_.emplace_back(std::make_pair(obj, f));
        }
    }

    void remove_listener(void* obj)
    {
        auto it = remove_if(begin(listeners_), end(listeners_), [&](auto& x) { return x.first == obj; });
        if (it != end(listeners_)) {
            listeners_.erase(it);
        }
    }

    void send_event(M msg) const
    {
        for (auto&& e : listeners_) {
            (e.second)(e.first, msg);
        }
    }
};

} // namespace
#endif
