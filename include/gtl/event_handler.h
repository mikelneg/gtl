#ifndef OWOBOAXZZZSDFWWER_GTL_EVENT_HANDLER_H_
#define OWOBOAXZZZSDFWWER_GTL_EVENT_HANDLER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::coroutine::
    
    class event_handler;
-----------------------------------------------------------------------------*/

#include <vector>
#include <gtl/events.h>
#include <boost/coroutine/asymmetric_coroutine.hpp>
#include <vn/boost_coroutine_utilities.h>
                                                   
namespace gtl {

namespace coroutine {

    class event_handler {        
        using coro = boost::coroutines::asymmetric_coroutine<gtl::event>;                
        coro::push_type coroutine_;
    public:

        using push_type = coro::push_type;
        using pull_type = coro::pull_type;

        event_handler();

        template <typename F>
        event_handler(F func) : coroutine_{vn::coroutines::make_trycatch_coroutine(std::move(func))} {}

        template <typename F>
        void replace_handler(F func) {
            coroutine_ = coro::push_type{vn::coroutines::make_trycatch_coroutine(std::move(func))};
        }

        void dispatch_event(gtl::event const&);
    };
                 

}} // namespace
#endif