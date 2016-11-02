/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef RYUWOFWOABF_GTL_STAGE_H_
#define RYUWOFWOABF_GTL_STAGE_H_

#include <gtl/d3d_types.h>
#include <gtl/synchronization_object.h>

//#include <boost/coroutine2/coroutine.hpp>
//#include <gtl/events.h>

#include <gtl/event_handler.h>

//#include <gtl/event_generator.h>

#include <gtl/rate_limiter.h>

#include <gtl/command_variant.h>
#include <gtl/scene.h>
#include <gtl/tags.h>

//#include <vector>
//#include <array>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <vn/tuple_utilities.h>
#include <vn/work_thread.h>

#include <gtl/audio_adapter.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <gtl/d3d_imgui_adapter.h>
#include <gtl/imgui_adapter.h>
#include <imgui.h>

#include <boost/container/flat_map.hpp>

namespace gtl {

class stage {

    struct resource_object {
        gtl::d3d::direct_command_allocator calloc_;
        gtl::d3d::graphics_command_list clist_before_;
        gtl::d3d::graphics_command_list clist_after_;
        resource_object(gtl::d3d::device&& dev) : calloc_{dev}, clist_before_{dev, calloc_}, clist_after_{dev, calloc_}
        {
        }
        resource_object(resource_object&&) = default;
        resource_object& operator=(resource_object&&) = default;
    };

    gtl::rate_limiter frame_rate_limiter_;
    std::vector<resource_object> buffered_resource_;
    std::vector<ID3D12CommandList*> draw_queue_;
    // gtl::d3d::synchronization_object synchronizer_;
    gtl::d3d::frame_synchronizer synchronizer_;

    vn::work_thread draw_thread_;

    gtl::coroutine::event_handler event_handler_;

    gtl::imgui_adapter mutable imgui_adapter_;
    gtl::d3d::imgui_adapter imgui_;

    // gtl::event_generator<gtl::command_variant> event_generator_;

public:
    template <typename SceneType>
    stage(gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType&, gtl::win::audio_adapter& audio);

    stage(stage&&) = delete;
    stage& operator=(stage&&) = delete;

    void present(gtl::d3d::swap_chain&, DXGI_PRESENT_PARAMETERS = {});
    void discard_frame_and_synchronize(gtl::d3d::swap_chain&);

    void dispatch_event(gtl::event const& e)
    {
        event_handler_.dispatch_event(e);
    }

    template <typename E>
    void dispatch_events(E const& events)
    {
        for (auto&& e : events)
        {
            event_handler_.dispatch_event(e);
            imgui_adapter_.dispatch_event(e);
        }

        // imgui_adapter_.render();
    }
};

template <typename SceneType>
stage::stage(gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue_, unsigned num_buffers, SceneType& scene_, gtl::win::audio_adapter& audio)
    : frame_rate_limiter_{std::chrono::milliseconds(15)},
      buffered_resource_{[&]() {
          std::vector<resource_object> tmp;
          for (unsigned i = 0; i < num_buffers; ++i)
          {
              tmp.emplace_back(get_device(swchain_));
          }
          return tmp;
      }()},
      synchronizer_{cqueue_, num_buffers - 1, static_cast<unsigned>((std::max)(0, static_cast<int>(num_buffers) - 2))},
      draw_thread_{[&, this](auto& frame_state_) {
                       synchronizer_(
                           [&](auto& frame_index) {
                               auto const current_index = value(frame_index);

                               assert(current_index < buffered_resource_.size());

                               resource_object& ro_ = buffered_resource_[current_index];

                               gtl::d3d::direct_command_allocator& calloc = ro_.calloc_;
                               gtl::d3d::graphics_command_list& clb = ro_.clist_before_;
                               gtl::d3d::graphics_command_list& cla = ro_.clist_after_;

                               calloc->Reset();

                               clb->Reset(calloc.get(), nullptr);

                               clb->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swchain_.get_current_resource(), D3D12_RESOURCE_STATE_PRESENT,
                                                                                             D3D12_RESOURCE_STATE_RENDER_TARGET));

                               clb->Close();
                               cla->Reset(calloc.get(), nullptr);

                               cla->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swchain_.get_current_resource(),
                                                                                             D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
                               cla->Close();

                               draw_queue_.clear();
                               draw_queue_.emplace_back(clb.get());

                               // stage_.draw_callback(...);
                               // scene_.draw_callback([&](auto f) { f(draw_queue_, static_cast<int>(current_index), 1.0f,
                               // swchain_.rtv_heap()); });
                               scene_.draw_callback([&](auto f) { f(draw_queue_, static_cast<int>(current_index), 1.0f, swchain_.get_current_handle()); });

                               imgui_.draw(draw_queue_, static_cast<int>(current_index), 1.0f, swchain_.get_current_handle());

                               draw_queue_.emplace_back(cla.get());

                               //
                               cqueue_->ExecuteCommandLists(static_cast<unsigned>(draw_queue_.size()), draw_queue_.data());

                               async_advance(frame_index);   // we advance the frame index..
                               make_available(frame_state_); // then we make the frame available
                           },
                           []() {});
                   },
                   []() {}},
      event_handler_{[&](auto& yield) {
          auto handler_ = vn::make_composite_function([&](gtl::commands::get_swap_chain, auto&& f) { f(swchain_); },
                                                      [&](gtl::commands::resize r, auto&&) {
                                                          draw_thread_.when_available([&](auto& frame_state_) {

                                                              discard_frame_and_synchronize(swchain_);

                                                              auto dims = swchain_.resize(r.w, r.h);
                                                              scene_.resize(dims.first, dims.second, cqueue_);

                                                              imgui_.resize(dims.first, dims.second, cqueue_);

                                                              // event_generator_.send_event(gtl::commands::resize);

                                                              consume_and_notify(frame_state_); // screws up the present()
                                                          });

                                                      },
                                                      [&](gtl::commands::get_some_resource, auto&& f) {
                                                          // std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
                                                          f([]() { std::cout << "look at me, all fancy..\n"; });
                                                      },
                                                      [&](gtl::commands::get_audio_adapter, auto&& f) {
                                                          // std::unique_lock<std::mutex> lock_{draw_thread_mutex_};
                                                          f(audio);
                                                      },
                                                      [](auto&&, auto&&) {});

          scene_.handle_events(handler_, yield);
      }},
      imgui_adapter_{std::unordered_map<std::string, std::function<void()>>{{"phys_debug", [&]() { scene_.toggle_debug(); }}}},
      imgui_{swchain_, cqueue_, imgui_adapter_}
{
    assert(num_buffers > 1);
    event_handler_.dispatch_event(gtl::events::none{});

    // scene_.attach_listener(event_generator_);

    // imgui_adapter_.render();
}

} // namespace
#endif
