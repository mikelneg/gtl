/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_
#define LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_

#include <gtl/d3d_types.h>
#include <gtl/events.h>
#include <gtl/keyboard_enum.h>

#include <gtl/command_variant.h>
#include <gtl/d3d_imgui_adapter.h>
#include <gtl/swirl_effect_transition_scene.h>

#include <gtl/physics_simulation.h>
#include <gtl/swap_vector.h>
#include <vn/swap_object.h>
#include <vn/boost_variant_utilities.h>

//#include <gtl/event_listener.h>

#include <vn/math_utilities.h>
#include <vn/single_consumer_queue.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <atomic>
#include <cmath>
#include <functional>
#include <vector>

#include <gtl/camera.h>

#include <GamePad.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>

namespace gtl {
namespace scenes {

    class main_scene {

        gtl::swap_vector<gtl::physics::generator> mutable physics_task_queue_;

        gtl::physics_simulation physics_;
        gtl::camera physics_camera_;

        gtl::physics::length<float> mutable camera_height_;

        gtl::scenes::transitions::swirl_effect swirl_effect_;

        gtl::imgui_adapter mutable imgui_adapter_;
        gtl::d3d::imgui_adapter imgui_;

        std::atomic<uint32_t> mutable current_id_{};

        std::atomic<int> mutable focus_id_{};

        vn::single_consumer_queue<std::function<void()>> mutable draw_task_queue_;

    public:
        main_scene(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue)
            : physics_task_queue_{[]() {
                  using namespace gtl::physics::generators;
                  using namespace boost::units;
                  std::vector<gtl::physics::generator> generators_;

                  generators_.emplace_back(static_box{{0.0f * si::meters, -100.0f * si::meters},
                                                      {200.0f * si::meters, 5.0f * si::meters},
                                                      0.0f * si::radians,
                                                      {}});
                  generators_.emplace_back(static_box{{0.0f * si::meters, 100.0f * si::meters},
                                                      {200.0f * si::meters, 5.0f * si::meters},
                                                      0.0f * si::radians,
                                                      {}});
                  generators_.emplace_back(static_box{{-100.0f * si::meters, 0.0f * si::meters},
                                                      {5.0f * si::meters, 200.0f * si::meters},
                                                      0.0f * si::radians,
                                                      {}});
                  generators_.emplace_back(static_box{{100.0f * si::meters, 0.0f * si::meters},
                                                      {5.0f * si::meters, 200.0f * si::meters},
                                                      0.0f * si::radians,
                                                      {}});

                  for (unsigned j = 0; j < 80; ++j)
                  {
                      std::vector<dynamic_box> jointed_boxes_;
                      auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                      auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                      auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                      for (unsigned i = 0; i < 4; ++i)
                      {
                          jointed_boxes_.emplace_back(
                              dynamic_box{{x, y - ((i * 1.0f) * si::meter)},
                                          {1.0f * si::meter, 1.0f * si::meter},
                                          angle,
                                          InstanceInfo{}.pack_entity_id(j + 600).pack_mesh_id(0)});
                      }
                      generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_)});
                  }

                  for (unsigned j = 80; j < 160; ++j)
                  {
                      std::vector<dynamic_box> jointed_boxes_;
                      auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                      auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                      auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                      // for (unsigned i = 0; i < 4; ++i) {
                      //  jointed_boxes_.emplace_back(
                      generators_.emplace_back(dynamic_box{{x, y},
                                                           {1.0f * si::meter, 1.0f * si::meter},
                                                           angle,
                                                           InstanceInfo{}.pack_entity_id(j + 600).pack_mesh_id(1)});
                      //}
                      // generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_)});
                  }

                  return generators_;
              }()},
              physics_camera_{{0.0f * boost::units::si::meters, 0.0f * boost::units::si::meters},
                              {1.0f * boost::units::si::meters, 1.0f * boost::units::si::meters},
                              gtl::physics::angle<float>{45.0f * boost::units::degree::degree},
                              {0.001f * boost::units::si::meters},
                              {100.0f * boost::units::si::meters}},
              camera_height_{10.0f * boost::units::si::meters},
              physics_{physics_task_queue_},
              swirl_effect_{dev, swchain, cqueue, physics_},
              imgui_adapter_{},
              imgui_{dev, swchain, cqueue, imgui_adapter_}
        {
            imgui_adapter_.render();
        }

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const
        {
            func([&](auto&&... ps) {
                Eigen::Matrix4f cam_transform_
                    = Eigen::Affine3f{Eigen::Scaling(1.0f / (camera_height_ / boost::units::si::meter)) //}.matrix();
                                      * Eigen::AngleAxisf{0.2f, Eigen::Vector3f{0.0f, 0.0f, -1.0f}}
                                      * Eigen::AngleAxisf{0.2f, Eigen::Vector3f{-1.0f, 0.0f, 0.0f}}}
                          .matrix();

                draw_task_queue_.consume([](auto&& f) { f(); }); // execute our waiting tasks..
                swirl_effect_.draw(ps..., current_id_, cam_transform_ * physics_camera_.matrix());
                imgui_adapter_.render();
                imgui_.draw(ps...);
            });
        }

        void resize(int w, int h, gtl::d3d::command_queue& c)
        {
            swirl_effect_.resize(w, h, c);
            imgui_.resize(w, h, c);
        }

        template <typename YieldType>
        gtl::event resizing_event_context(YieldType& yield) const
        {
            // timer begins
            // yes is pressed, no is pressed, esc is pressed, timer expires
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;

            while (!same_type(yield().get().value(), ev::exit_immediately{}))
            {
                if (same_type(yield.get().value(), ev::keydown{}))
                {
                    switch (boost::get<ev::keydown>(yield.get().value()).key)
                    {
                        case k::Escape: // falls through..
                        case k::N:
                            return gtl::events::revert{};
                            break;

                        case k::Y:
                            return gtl::events::keep{};
                            break;
                        default:
                            break;
                    }
                }
            }
            return gtl::events::revert{};
        }

        template <typename ResourceManager, typename YieldType>
        gtl::event handle_events(ResourceManager& resource_callback_, YieldType& yield) const
        {
            // HACK tinkering.. change how things are dispatched..

            namespace ev = gtl::events;
            namespace k = gtl::keyboard;
            using namespace boost::units;
            using namespace gtl::physics::generators;

            int count{};

            std::vector<gtl::physics::generator> task_local_;

            uint16_t selected_id{};
            bool shift_down_{};

            bool quit_{};
            // gtl::events::event_variant quit_event_{ev::none{}}

            auto mouse_handler = vn::make_lambda_visitor(
                [&](ev::mouse_wheel_scroll const& e) {
                    std::cout << "mouse scroll : new delta = " << e.wheel_delta << ", keystate == " << e.key_state
                              << "\n";
                    camera_height_ += (e.wheel_delta > 0 ? -0.1f : 0.1f) * boost::units::si::meter;
                    std::cout << "camera's new height == " << camera_height_ / boost::units::si::meter << "\n";
                },
                [&](ev::mouse_lbutton_down const& e) {
                    selected_id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "selected object " << selected_id << "\n";

                    int mx = GET_X_LPARAM(e.coord); // TODO replace this coordinate stuff..
                    int my = GET_Y_LPARAM(e.coord);

                    draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_down(mx, my); });
                    // resource_callback_(gtl::commands::get_audio_adapter{}, [](auto&& aud) { aud.play_effect("click");
                    // }); // TODO causes an ICE..
                },
                [&](ev::mouse_rbutton_down const& e) {

                    selected_id = current_id_.load(std::memory_order_relaxed);

                    if (shift_down_)
                    {
                        task_local_.emplace_back(destroy_object_implode{selected_id});
                        physics_task_queue_.swap_in(task_local_); // TODO fix how tasks are submitted..
                    }
                    else
                    {
                        task_local_.emplace_back(boost_object{selected_id});
                        physics_task_queue_.swap_in(task_local_);
                    }
                },
                [&](ev::mouse_lbutton_up const& e) {
                    int mx = GET_X_LPARAM(e.coord);
                    int my = GET_Y_LPARAM(e.coord);
                    draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_up(mx, my); });
                },
                [&](ev::mouse_moved const& e) {
                    swirl_effect_.set_mouse_coords(e.coord);

                    int mx = GET_X_LPARAM(e.coord);
                    int my = GET_Y_LPARAM(e.coord);
                    draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_move(mx, my); });
                },
                [](auto const&) { // empty catch-all
                });

            auto quit_handler = [&](gtl::events::event_variant) { quit_ = true; };

            auto top_level_handler = vn::make_lambda_visitor(
                [&](ev::mouse_event const& m) { mouse_handler(m); },
                [&](ev::exit_immediately const&) {
                    quit_ = true;
                    quit_handler(ev::exit_all{});
                },
                [&](ev::keydown const& k) {
                    switch (k.key)
                    {
                        case k::Shift:
                            shift_down_ = true;
                            break;
                        case k::Escape:
                        {
                            std::cout << "swirl_effect(): escape pressed, exiting all..\n";
                            quit_handler(ev::exit_all{});
                        }
                        break;
                        case k::Q:
                        {
                            std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count
                                      << ")\n";
                            quit_handler(ev::exit_state{0});
                        }
                        break;
                        case k::A:
                        {
                            std::cout << "swirl_effect(): A pressed, generating new object.. \n";
                            task_local_.emplace_back(dynamic_box{{0.0f * si::meter, 0.0f * si::meter},
                                                                 {0.5f * si::meter, 0.5f * si::meter},
                                                                 0.0f * si::radians,
                                                                 InstanceInfo{}.pack_entity_id(888)});
                            physics_task_queue_.swap_in(task_local_);
                        }
                        break;
                        case k::K:
                        {
                            std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";
                            throw std::runtime_error{__func__};
                        }
                        break;
                        case k::R:
                        {
                            std::cout << "swirl_effect() : r pressed, issuing resize..\n";
                            std::pair<int, int> dims{};

                            // TODO resource_callback_() calls are producing ICEs...

                            // resource_callback_(gtl::commands::get_swap_chain{}, [&](auto& s) { dims = s.dimensions();
                            // });
                            resource_callback_(gtl::commands::resize{1280, 720}, []() {});

                            // std::cout << "entering resize context..\n";
                            // if (same_type(resizing_event_context(yield), ev::keep{}))
                            //{
                            //    std::cout << "keeping new size..\n";
                            //}
                            // else
                            //{
                            //    std::cout << "reverting to old size..\n";
                            //    resource_callback_(gtl::commands::resize{dims.first, dims.second}, []() {});
                            //}
                        }
                        break;
                        default:
                        {
                            std::cout << "swirl_effect() : unknown key pressed\n";
                        }
                        break;
                    }
                },
                [&](ev::keyup const& k) {
                    switch (k.key)
                    {
                        case k::Shift:
                        {
                            shift_down_ = false;
                        }
                        break;
                        default:
                            break;
                    }
                },
                [&](ev::dpad_pressed const& e) {
                    task_local_.emplace_back(boost_object_vec{selected_id, e.x / 400.0f, e.y / 400.0f}); // HACK
                    physics_task_queue_.swap_in(task_local_);
                },
                [&](ev::resize_swapchain const&) {}, [](auto const&) {});

            while (!quit_)
            {
                top_level_handler(yield().get().value());
            }

            return ev::exit_state{0};
        }
    };
}
} // namespaces
#endif
