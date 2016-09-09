#include "gtl/event_handler.h"

#include <vn/boost_coroutine_utilities.h>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace coroutine {

    event_handler::event_handler()
        : coroutine_{ vn::coroutines::make_empty_coroutine() }
    {
    }

    void event_handler::dispatch_event(gtl::event const& e)
    {
        vn::coroutines::try_invoke(coroutine_, e);
    }

    //void event_handler::enqueue_event(gtl::event e)
    //{
    //    event_queue_.emplace_back(std::move(e));
    //}
    //
    //void event_handler::publish_event_immediately(gtl::event e)
    //{
    //    vn::coroutines::try_invoke(coroutine_,std::move(e));
    //}
    //
    //void event_handler::publish_events()
    //{
    //    vn::coroutines::try_invoke(coroutine_,gtl::events::none{});
    //    for (auto&& e : event_queue_) {
    //        vn::coroutines::try_invoke(coroutine_,std::move(e));
    //    }
    //    event_queue_.clear();
    //}
}
} // namespaces
