/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/stage.h"

#include <gtl/events.h>
//#include <gtl/scenes.h>

#include <gtl/d3d_helper_funcs.h>
#include <gtl/d3d_types.h>
#include <gtl/gtl_window.h>

#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>

#include <vn/boost_visitors.h>

#include <chrono>
#include <functional>
#include <future>
#include <utility>

#include <algorithm>
#include <cassert>
#include <unordered_map>

#include <vn/composite_function.h>
#include <vn/scope_guard.h>
//#include <gtl/scenes.h> // TODO remove
#include <gtl/command_variant.h>
#include <gtl/main_scene.h>

#include <gtl/scene.h>
#include <gtl/tags.h>

#include <gtl/events.h>
#include <vn/work_thread.h>

namespace gtl {

namespace {

    struct empty_scene {

        // empty_scene(empty_scene const&) = default;
        // empty_scene& operator=(empty_scene const&) = default;

        template <typename... Ts>
        constexpr empty_scene(Ts&&...) noexcept
        {
        }

        template <typename... Ts>
        void operator()(Ts const&...) const
        {
        }
    };
}

void stage::present(gtl::d3d::swap_chain& swchain_, DXGI_PRESENT_PARAMETERS dxgi_pp)
{
    frame_rate_limiter_([&swchain_, &dxgi_pp, this]() {

        imgui_adapter_.render();

        draw_thread_.if_available(
            [&](auto& frame_state_) { // (auto& state_){  // consumes itself if not consumed..
                swchain_->Present1(0, 0, std::addressof(dxgi_pp)); // potentially blocking
                consume_and_notify(frame_state_);
            },
            [] {});

    });
}

void stage::discard_frame_and_synchronize(gtl::d3d::swap_chain& swchain_)
{
    DXGI_PRESENT_PARAMETERS p{};

    //    auto waitable = swchain_->GetFrameLatencyWaitableObject();

    swchain_->Present1(0, DXGI_PRESENT_RESTART, std::addressof(p));

    //  WaitForSingleObject(waitable,INFINITE);
}

} // namespace
