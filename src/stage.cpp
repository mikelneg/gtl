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

        //empty_scene(empty_scene const&) = default;
        //empty_scene& operator=(empty_scene const&) = default;

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

//    template <typename S>
//static void draw_thread_thread(std::mutex& draw_thread_mutex_,
//                        bool& quit_flag_,
//                        std::condition_variable& cv_,
//                        std::atomic<stage::sig>& frame_state_,
//
//    gtl::d3d::device dev_, gtl::d3d::swap_chain& swchain_,
//                        gtl::d3d::command_queue& cqueue_, unsigned num_buffers, S& main_scene_);
//

//stage::stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
//    :  // dev_{get_device(swchain)},
//       // swchain_{swchain},
//       // cqueue_{cqueue_},
//        //synchronizer_{cqueue_, num_buffers-1, (std::max)(0,static_cast<int>(num_buffers)-2)}, // maximum desync value.
//        scene_{empty_scene{}},
//        event_handler_{},
//        //scenes_{},
//        //scenes_{},
//        //dxgi_pp{},
//        //scene_builder_{[&](auto& scene_graph_, auto& yield_, auto& mutex_){
//         //                   scene_graph_.transition_scene(yield_, get_device(swchain), cqueue_, swchain, mutex_);
//         //             }},
//        quit_flag_{false},
//        frame_rate_limiter_{std::chrono::milliseconds(9)} // frame time limit.. 17 == ~60fps, 9 == ~120fps
//        //draw_params_{draw_queue_, 0, 0.0f, swchain.rtv_heap()}
//{
//        assert(num_buffers > 1);
//        frame_state_.store(sig::frame_consumed);
//        //quit_flag_.test_and_set();
//
//        event_handler_.exchange_handler(
//        [&](auto& yield){
//            gtl::scenes::main_scene main_scene_{get_device(swchain),swchain,cqueue_};
//
//            auto callback_handler_ =
//            vn::make_composite_function(
//                [&](gtl::commands::get_swap_chain, auto&& f)
//                {
//                    f(swchain_);
//                },
//                [&](gtl::commands::get_some_resource, auto&& f) {
//                    std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
//                    f([](){ std::cout << "look at me, all fancy..\n"; });
//                }
//            );
//
//            draw_thread_thread_ = std::thread{&draw_thread_thread<decltype(main_scene_)>,
//                                        std::ref(draw_thread_mutex_),
//                                        std::ref(quit_flag_),
//                                        std::ref(cv_),
//                                        std::ref(frame_state_),
//                                        get_device(swchain),
//                                        std::ref(swchain),
//                                        std::ref(cqueue_),
//                                        num_buffers,
//                                        std::ref(main_scene_)
//                                   };
//
//            main_scene_.handle_events(callback_handler_,yield);
//        });
//
//        event_handler_.dispatch_event(gtl::events::none{});
//        //draw_thread_thread_ = std::thread{&stage::draw_thread_thread,
//        //                           this,
//        //                                get_device(swchain),
//        //                                std::ref(swchain),
//        //                                std::ref(cqueue_),
//        //                                num_buffers
//        //                           };
//}

//stage::~stage()
//{
//    //{
//    //    std::unique_lock<std::mutex> lock{draw_thread_mutex_};
//    //    //quit_flag_.clear(std::memory_order_release);
//    //    quit_flag_ = true;
//    //    frame_state_.store(sig::frame_consumed,std::memory_order_release);
//    //    cv_.notify_all();
//    //}
//    //if (draw_thread_thread_.joinable()) {
//    //    draw_thread_thread_.join();
//    //}
//}

//namespace { // implementation detail..
//    struct resource_object {
//        gtl::d3d::direct_command_allocator calloc_;
//        gtl::d3d::graphics_command_list clist_before_;
//        gtl::d3d::graphics_command_list clist_after_;
//        resource_object(gtl::d3d::device& dev) : calloc_{dev}, clist_before_{dev,calloc_}, clist_after_{dev,calloc_} {}
//        resource_object(resource_object&&) = default;
//        resource_object& operator=(resource_object&&) = default;
//    };
//
//    template <typename T>
//    using id_t = T;
//
//    //template <typename ...Ts, typename ...Qs>
//    //static
//    //void assign_draw_params(std::tuple<Ts...>& tuple_, Qs&&...qs) {
//    //    tuple_ = {qs...};
//    //}
//}

//void stage::draw_thread_thread(gtl::d3d::device dev_, gtl::d3d::swap_chain& swchain_,
//                        gtl::d3d::command_queue& cqueue_, unsigned num_buffers)
//
//{
//    std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
//
//    //
//    std::vector<resource_object> buffered_resource_;
//    buffered_resource_.reserve(num_buffers);
//
//    for (unsigned i = 0; i < num_buffers; ++i) {
//        buffered_resource_.emplace_back(dev_);
//    }
//    //
//
//
//
//    //auto callback_handler_ =
//    //vn::make_composite_function(
//    //    [&](gtl::commands::get_swap_chain, auto&& f)
//    //    {
//    //        f(swchain_);
//    //    },
//    //    [&](gtl::commands::get_some_resource, auto&& f) {
//    //        std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
//    //        f([](){ std::cout << "look at me, all fancy..\n"; });
//    //    }
//    //);
//
//    //auto callback_ultimate_handler_ =
//    //vn::make_composite_function(
//    //    [&](auto&& c, auto&& f) {
//    //        auto k = [&](auto const& v) { callback_handler_(v,f); };
//    //        boost::apply_visitor(k,c);
//    //    });
//
//
//    //gtl::scenes::main_scene main_scene_{dev_,swchain_,cqueue_};
//
//    auto&& scope_guard = vn::make_scope_guard(
//        [&](){  event_handler_.exchange_handler([&](auto& yield){ main_scene_.handle_events(callback_handler_, yield); }); },
//        [&](){  event_handler_.exchange_handler([&](auto& yield){ }); });
//
//    gtl::d3d::synchronization_object synchronizer_{cqueue_, num_buffers-1,
//                                            static_cast<unsigned>((std::max)(0,static_cast<int>(num_buffers)-2))};
//
//
//
//    //while(quit_flag_.test_and_set(std::memory_order_acq_rel)) {
//    while(!quit_flag_) {
//        cv_.wait(lock_, [this](){
//            return frame_state_.load(std::memory_order_acquire) == sig::frame_consumed;
//        });
//
//        //if (!quit_flag_.test_and_set(std::memory_order_acq_rel)) { return; }
//        if (quit_flag_) { return; }
//
//        synchronizer_([&buffered_resource_,&cqueue_,&swchain_,&main_scene_,this](auto& sync_index) {
//            assert(value(sync_index) < buffered_resource_.size());
//            resource_object& ro_ = buffered_resource_[value(sync_index)];
//            gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
//            gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
//            gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
//
//            auto res1 = calloc->Reset();
//            (void)res1;
//            auto res2 = clb->Reset(calloc.get(), nullptr);
//            (void)res2;
//
//            clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                              swchain_.get_current_resource(),
//                                              D3D12_RESOURCE_STATE_PRESENT,
//                                              D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//            clb->Close();
//
//            auto res3 = cla->Reset(calloc.get(), nullptr);
//            (void)res3;
//
//            cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                              swchain_.get_current_resource(),
//                                              D3D12_RESOURCE_STATE_RENDER_TARGET,
//                                              D3D12_RESOURCE_STATE_PRESENT));
//            cla->Close();
//
//            std::vector<ID3D12CommandList*> &v = draw_queue_;
//
//            v.clear();
//
//            v.emplace_back(clb.get());
//
//            // TODO rethink this call..
//
//
//
//            //assign_draw_params(draw_params_, v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap());
//
//            //scene_.send_command(gtl::commands::draw{});
//            // causes scene_ to call draw_callback with a functional like [&](auto&&...ps){ scene_internal_.draw(ps...); }
//            // draw_callback then calls that functional with draw_params_ expanded
//
//            // could also be something like
//
//            // scene_.draw_callback([&](auto& f){ f(v, value(sync_index), 1.0f, swchain_.rtv_heap()); });
//            // which does
//            // void draw_callback(func f) {
//            //      f([&](auto&&...ps){  draw_a(ps...); draw_b(ps...); }); }
//            //
//            // or ..
//            // scene_.draw_callback([&](auto& f, int ndx){ f(v,value(sync_index), 1.0f, swchain_, viewport[ndx]); });
//            // void draw_callback(func g) {
//            //      g([&](auto&&...ps){ draw_a(ps...); }, a_index);
//            //      g([&](auto&&...ps){ draw_b(ps...); }, b_index); }
//
//            main_scene_.draw_callback([&](auto f) { f(v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); });
//
//            // // modifying to work with attach_scene ..
//            //auto vec = boost::apply_visitor([&](auto& scene){
//            //    return scene.draw(static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); // draw call..
//            //},scenes_.current_scene());
//
//            //current_scene_.send_command(gtl::commands::draw{});
//
//            //for (auto&& e : vec) v.emplace_back(e);
//            v.emplace_back(cla.get());
//
//            //
//            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());
//            advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready
//            frame_state_.store(sig::frame_ready,std::memory_order_release);
//       },[](){});
//    }
//}

//template <typename S>
//static void draw_thread_thread(std::mutex& draw_thread_mutex_,
//                        bool& quit_flag_,
//                        std::condition_variable& cv_,
//                        std::atomic<stage::sig>& frame_state_,
//                        gtl::d3d::device dev_, gtl::d3d::swap_chain& swchain_,
//                        gtl::d3d::command_queue& cqueue_, unsigned num_buffers, S& main_scene_)
//
//{
//    using sig = stage::sig;
//
//
//    std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
//
//    //
//    std::vector<resource_object> buffered_resource_;
//    buffered_resource_.reserve(num_buffers);
//
//    for (unsigned i = 0; i < num_buffers; ++i) {
//        buffered_resource_.emplace_back(dev_);
//    }
//    //
//
//
//
//    //auto callback_handler_ =
//    //vn::make_composite_function(
//    //    [&](gtl::commands::get_swap_chain, auto&& f)
//    //    {
//    //        f(swchain_);
//    //    },
//    //    [&](gtl::commands::get_some_resource, auto&& f) {
//    //        std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
//    //        f([](){ std::cout << "look at me, all fancy..\n"; });
//    //    }
//    //);
//
//    //auto callback_ultimate_handler_ =
//    //vn::make_composite_function(
//    //    [&](auto&& c, auto&& f) {
//    //        auto k = [&](auto const& v) { callback_handler_(v,f); };
//    //        boost::apply_visitor(k,c);
//    //    });
//
//
//    //gtl::scenes::main_scene main_scene_{dev_,swchain_,cqueue_};
//
//    //auto&& scope_guard = vn::make_scope_guard(
//    //    [&](){  event_handler_.exchange_handler([&](auto& yield){ main_scene_.handle_events(callback_handler_, yield); }); },
//    //    [&](){  event_handler_.exchange_handler([&](auto& yield){ }); });
//
//    gtl::d3d::synchronization_object synchronizer_{cqueue_, num_buffers-1,
//                                            static_cast<unsigned>((std::max)(0,static_cast<int>(num_buffers)-2))};
//
//    std::vector<ID3D12CommandList*> draw_queue_;
//
//    //while(quit_flag_.test_and_set(std::memory_order_acq_rel)) {
//    while(!quit_flag_) {
//        cv_.wait(lock_, [&](){
//            return frame_state_.load(std::memory_order_acquire) == sig::frame_consumed;
//        });
//
//        //if (!quit_flag_.test_and_set(std::memory_order_acq_rel)) { return; }
//        if (quit_flag_) { return; }
//
//        //synchronizer_([&buffered_resource_,&cqueue_,&swchain_,&main_scene_](auto& sync_index) {
//        synchronizer_([&](auto& sync_index) {
//            assert(value(sync_index) < buffered_resource_.size());
//            resource_object& ro_ = buffered_resource_[value(sync_index)];
//            gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
//            gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
//            gtl::d3d::graphics_command_list& cla = ro_.clist_after_;
//
//            auto res1 = calloc->Reset();
//            (void)res1;
//            auto res2 = clb->Reset(calloc.get(), nullptr);
//            (void)res2;
//
//            clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                              swchain_.get_current_resource(),
//                                              D3D12_RESOURCE_STATE_PRESENT,
//                                              D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//            clb->Close();
//
//            auto res3 = cla->Reset(calloc.get(), nullptr);
//            (void)res3;
//
//            cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                              swchain_.get_current_resource(),
//                                              D3D12_RESOURCE_STATE_RENDER_TARGET,
//                                              D3D12_RESOURCE_STATE_PRESENT));
//            cla->Close();
//
//            std::vector<ID3D12CommandList*> &v = draw_queue_;
//
//            v.clear();
//
//            v.emplace_back(clb.get());
//
//            // TODO rethink this call..
//
//
//
//            //assign_draw_params(draw_params_, v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap());
//
//            //scene_.send_command(gtl::commands::draw{});
//            // causes scene_ to call draw_callback with a functional like [&](auto&&...ps){ scene_internal_.draw(ps...); }
//            // draw_callback then calls that functional with draw_params_ expanded
//
//            // could also be something like
//
//            // scene_.draw_callback([&](auto& f){ f(v, value(sync_index), 1.0f, swchain_.rtv_heap()); });
//            // which does
//            // void draw_callback(func f) {
//            //      f([&](auto&&...ps){  draw_a(ps...); draw_b(ps...); }); }
//            //
//            // or ..
//            // scene_.draw_callback([&](auto& f, int ndx){ f(v,value(sync_index), 1.0f, swchain_, viewport[ndx]); });
//            // void draw_callback(func g) {
//            //      g([&](auto&&...ps){ draw_a(ps...); }, a_index);
//            //      g([&](auto&&...ps){ draw_b(ps...); }, b_index); }
//
//            main_scene_.draw_callback([&](auto f) { f(v, static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); });
//
//            // // modifying to work with attach_scene ..
//            //auto vec = boost::apply_visitor([&](auto& scene){
//            //    return scene.draw(static_cast<int>(value(sync_index)), 1.0f, swchain_.rtv_heap()); // draw call..
//            //},scenes_.current_scene());
//
//            //current_scene_.send_command(gtl::commands::draw{});
//
//            //for (auto&& e : vec) v.emplace_back(e);
//            v.emplace_back(cla.get());
//
//            //
//            cqueue_->ExecuteCommandLists(static_cast<unsigned>(v.size()),v.data());
//            synchronous_advance(sync_index);    // the command queue's update has to be sequenced before the frame is made ready
//            frame_state_.store(sig::frame_ready,std::memory_order_release);
//       },[](){});
//    }
//}
//

void stage::present(gtl::d3d::swap_chain& swchain_, DXGI_PRESENT_PARAMETERS dxgi_pp)
{
    frame_rate_limiter_([&swchain_, &dxgi_pp, this]() {

        draw_thread_.if_available([&](auto& frame_state_) {    // (auto& state_){  // consumes itself if not consumed..
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

//void stage::event_handler(coro::pull_type& yield)
//{
//}

} // namespace
