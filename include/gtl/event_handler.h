/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef OWOBOAXZZZSDFWWER_GTL_EVENT_HANDLER_H_
#define OWOBOAXZZZSDFWWER_GTL_EVENT_HANDLER_H_

#include <boost/coroutine/asymmetric_coroutine.hpp>
#include <gtl/events.h>
#include <vector>
#include <vn/boost_coroutine_utilities.h>

namespace gtl {

namespace coroutine {

    class event_handler {
        using coro = boost::coroutines::asymmetric_coroutine<gtl::event>;

        class scoped_replacement {
            // TODO swaps coroutines; on scope exit swaps back..
        };

        coro::push_type coroutine_;

    public:
        using push_type = coro::push_type;
        using pull_type = coro::pull_type;

        event_handler();

        template <typename F>
        event_handler(F func) : coroutine_{vn::coroutines::make_trycatch_coroutine(std::move(func))}
        {
        }

        template <typename F>
        auto exchange_handler(F func)
        {
            auto ret = std::move(coroutine_);
            coroutine_ = coro::push_type{vn::coroutines::make_trycatch_coroutine(std::move(func))};
            return ret;
        }

        auto exchange_handler(coro::push_type&& other)
        {
            auto ret = std::move(coroutine_);
            coroutine_ = std::move(other);
            return ret;
        }

        void dispatch_event(gtl::event const&);
    };
}
} // namespace
#endif