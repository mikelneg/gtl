/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_
#define LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_

#include <gtl/d3d_types.h>
#include <gtl/events.h>
#include <gtl/win_keyboard.h>

#include <gtl/command_variant.h>
#include <gtl/d3d_imgui_adapter.h>
#include <gtl/swirl_effect_transition_scene.h>

#include <gtl/d3d_draw_kit.h>
#include <gtl/draw_kit.h>

#include <gtl/physics/bullet/simulation_impl.h>
#include <gtl/physics/generators/simple_boundary.h>
#include <gtl/physics/common_types.h>
#include <gtl/physics/units.h>
#include <gtl/physics/command_variant.h>
#include <gtl/swap_vector.h>

#include <vn/swap_object.h>
#include <vn/boost_variant_utilities.h>

#include <gtl/camera.h>

//#include <gtl/event_listener.h>

#include <vn/math_utilities.h>
#include <vn/single_consumer_queue.h>

#include <boost/functional/hash.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <atomic>
#include <memory>
#include <cmath>
#include <functional>
#include <vector>

#include <GamePad.h>

//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//#include <imgui.h>

namespace gtl {
namespace scenes {

    class main_scene {        
        
        //template <typename T>
        //using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;

        vn::single_consumer_queue<gtl::physics::command_variant> mutable physics_task_queue_;

        gtl::draw_kit mutable draw_kit_;
        gtl::d3d::draw_kit box2d_;         

        std::unique_ptr<gtl::physics::simulation> physics_;
        gtl::camera physics_camera_;

        gtl::physics::length<float> mutable camera_height_;

        gtl::scenes::transitions::swirl_effect swirl_effect_;        
        std::atomic<uint32_t> mutable current_id_{};

        std::atomic<int> mutable focus_id_{};

        vn::single_consumer_queue<std::function<void()>> mutable draw_task_queue_;
        
        auto new_box_generator() {

            // mesh_group& g;
            // 
            // mesh_loader m{filename};
            //
            // g.add(m);
            // 
            // use(m.skeleton(),g.id(m)); 
            //
            // g.add(m);
            //
            //


        }

        auto box_generator() {
            using namespace gtl::physics::commands;
            using namespace boost::units;

            std::vector<std::pair<float, float>> grid; 
            for (int x = -20; x < 20; x++)
                for (int y = 0; y < 40; y++)
                    grid.emplace_back(x * 3.0f, y * 3.0f);

            std::vector<gtl::physics::command_variant> generators_;

            for (int i = 1; i < 200; ++i) {
                generators_.emplace_back(dynamic_box{ {vn::math::rand_neg_one_one() * 10.0f,
                                                       vn::math::rand_zero_one() * 10.0f,
                                                       vn::math::rand_zero_one() * 10.0f + 10.f}, 
                                                       {1.0f,1.0f,1.0f},
                                                       Eigen::Quaternionf::Identity(),
                                                       gtl::entity::render_data{}.pack_entity_id(i).pack_mesh_id(1),
                                                       gtl::entity::id(i),
                                                       10.0f * si::kilograms });
            }   
            
            for (unsigned i = 1; i < 100; ++i) {
                std::vector<dynamic_box> jointed_boxes_;
                auto head_x = vn::math::rand_neg_one_one() * 3.0f;
                auto head_y = vn::math::rand_zero_one() * 15.0f;
                auto head_z = vn::math::rand_zero_one() * 15.0f + 10.0f;
                //auto head_angle = 0.0f * si::radians; 
            
                for (unsigned j = 0; j < 4; ++j) {
                    jointed_boxes_.emplace_back(dynamic_box{ {head_x, head_y, head_z - (j * 1.0f)}, 
                                                             {1.0f,1.0f,1.0f},
                                                             Eigen::Quaternionf::Identity(),                                                             
                                                             gtl::entity::render_data{}.pack_entity_id(i+200).pack_mesh_id(0),
                                                             gtl::entity::id(i+200),
                                                             10.0f * si::kilograms});
                }
                generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_),
                                                               gtl::entity::render_data{}.pack_entity_id(i+200).pack_mesh_id(0),
                                                               gtl::entity::id(i+200)});
            }

            for (unsigned i = 1; i < 200; ++i) {
                std::vector<dynamic_box> jointed_boxes_;
                auto head_x = vn::math::rand_neg_one_one() * 3.0f;
                auto head_y = vn::math::rand_zero_one() * 15.0f;
                auto head_z = vn::math::rand_zero_one() * 15.0f + 10.0f;
                //auto head_angle = 0.0f * si::radians; 
            
                for (unsigned j = 0; j < 4; ++j) {
                    jointed_boxes_.emplace_back(dynamic_box{ {head_x, head_y, head_z - (j * 1.0f)}, 
                                                             {1.0f,1.0f,1.0f},
                                                             Eigen::Quaternionf::Identity(),                                                             
                                                             gtl::entity::render_data{}.pack_entity_id(i+600).pack_mesh_id(2),
                                                             gtl::entity::id(i+600),
                                                             10.0f * si::kilograms});
                }
                generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_),
                                                               gtl::entity::render_data{}.pack_entity_id(i+600).pack_mesh_id(2),
                                                               gtl::entity::id(i+600)});
            }

            return generators_;
        }

        auto default_generator()
        {
            using namespace gtl::physics::commands;
            using namespace boost::units;
            std::vector<gtl::physics::command_variant> generators_;

            generators_.emplace_back(
                polymorphic_generator{
                    std::make_unique<gtl::physics::generators::simple_boundary>(gtl::physics::position<float>{0.0f * si::meters, 0.0f * si::meters}, 
                                                                                gtl::physics::dimensions<float>{160.0f * si::meters, 160.0f * si::meters})
                 }
            );

            //generators_.emplace_back(static_rectangle{{0.0f * si::meters, -100.0f * si::meters}, {200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
            //generators_.emplace_back(static_rectangle{{0.0f * si::meters, 100.0f * si::meters}, {200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
            //generators_.emplace_back(static_rectangle{{-100.0f * si::meters, 0.0f * si::meters}, {5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
            //generators_.emplace_back(static_rectangle{{100.0f * si::meters, 0.0f * si::meters}, {5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});

            for (unsigned j = 0; j < 80; ++j)
            {
                std::vector<gtl::physics::commands::dynamic_rectangle> jointed_boxes_;
                auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                for (unsigned i = 0; i < 4; ++i)
                {
                    jointed_boxes_.emplace_back(gtl::physics::commands::dynamic_rectangle{{x, y - ((i * 1.0f) * si::meter)}, 
                                                {1.0f * si::meter, 1.0f * si::meter}, 
                                                angle, gtl::entity::render_data{}.pack_entity_id(j + 600).pack_mesh_id(0)});
                }
                generators_.emplace_back(gtl::physics::commands::dynamic_jointed_rectangles{std::move(jointed_boxes_)});
            }

            for (unsigned j = 80; j < 160; ++j)
            {
                std::vector<gtl::physics::commands::dynamic_rectangle> jointed_boxes_;
                auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                // for (unsigned i = 0; i < 4; ++i) {
                //  jointed_boxes_.emplace_back(
                generators_.emplace_back(gtl::physics::commands::dynamic_rectangle{{x, y}, {1.0f * si::meter, 1.0f * si::meter}, angle, gtl::entity::render_data{}.pack_entity_id(j + 600).pack_mesh_id(1)});
                //}
                // generators_.emplace_back(dynamic_jointed_rectangles{std::move(jointed_boxes_)});
            }

            return generators_;
        }

        auto test_generator()
        {
            using namespace gtl::physics::commands;
            using namespace boost::units;
            std::vector<gtl::physics::command_variant> generators_;

            generators_.emplace_back(polymorphic_generator{std::make_unique<gtl::physics::generators::simple_boundary>(
                gtl::physics::position<float>{0.0f * si::meters, 0.0f * si::meters}, 
                gtl::physics::dimensions<float>{150.0f * si::meters, 150.0f * si::meters}
                )});


//            generators_.emplace_back(static_rectangle{{0.0f * si::meters, -100.0f * si::meters}, {200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
//            generators_.emplace_back(static_rectangle{{0.0f * si::meters, 100.0f * si::meters}, {200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
//            generators_.emplace_back(static_rectangle{{-100.0f * si::meters, 0.0f * si::meters}, {5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
//            generators_.emplace_back(static_rectangle{{100.0f * si::meters, 0.0f * si::meters}, {5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});

            for (unsigned j = 0; j < 50; ++j)
            {
                std::vector<dynamic_rectangle> jointed_boxes_;
                auto x = vn::math::rand_neg_one_one() * 15.0f * si::meter;
                auto y = vn::math::rand_neg_one_one() * 15.0f * si::meter;
                auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                for (unsigned i = 0; i < 4; ++i)
                {
                    jointed_boxes_.emplace_back(dynamic_rectangle{
                        {x, y - ((i * 1.0f) * si::meter)}, {1.0f * si::meter, 1.0f * si::meter}, angle, gtl::entity::render_data{}.pack_entity_id(j + 600).pack_mesh_id(0)});
                }
                generators_.emplace_back(dynamic_jointed_rectangles{std::move(jointed_boxes_)});
            }

            for (unsigned j = 50; j < 100; ++j)
            {
                std::vector<dynamic_rectangle> jointed_boxes_;
                auto x = vn::math::rand_neg_one_one() * 15.0f * si::meter;
                auto y = vn::math::rand_neg_one_one() * 15.0f * si::meter;
                auto angle = 0.0f * si::radians; // vn::math::rand_neg_one_one() * si::radians;

                // for (unsigned i = 0; i < 4; ++i) {
                //  jointed_boxes_.emplace_back(
                generators_.emplace_back(dynamic_rectangle{{x, y}, {1.0f * si::meter, 1.0f * si::meter}, angle, gtl::entity::render_data{}.pack_entity_id(j + 600).pack_mesh_id(1)});
                //}
                // generators_.emplace_back(dynamic_jointed_rectangles{std::move(jointed_boxes_)});
            }

            return generators_;
        }

    public:
        main_scene(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue)
            : physics_task_queue_{box_generator()},
              physics_camera_{{1.0f * boost::units::si::meters, 1.0f * boost::units::si::meters},
                              gtl::physics::angle<float>{45.0f * boost::units::degree::degree},
                              {0.01f * boost::units::si::meters},
                              {2000.0f * boost::units::si::meters}},
              camera_height_{10.0f * boost::units::si::meters},
              physics_{std::make_unique<gtl::physics::bullet_simulation>(physics_task_queue_, draw_kit_)},
              swirl_effect_{dev, swchain, cqueue, *physics_},
              draw_kit_{},
              box2d_{dev, swchain, cqueue, draw_kit_}
        // imgui_adapter_{},
        // imgui_{dev, swchain, cqueue, imgui_adapter_}
        {
            // imgui_adapter_.render();
        }

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const
        {
            func([&](auto&&... ps) {
                Eigen::Matrix4f cam_transform_
                    = Eigen::Affine3f{  Eigen::Translation3f{0.0f,-(camera_height_ / boost::units::si::meter),60.0f} 
                                     //* Eigen::Scaling(1.0f / (camera_height_ / boost::units::si::meter))                                      
                                      * Eigen::AngleAxisf{vn::math::deg_to_rad( -20.0f), Eigen::Vector3f{1.0f, 0.0f, 0.0f}} 
                                      * Eigen::AngleAxisf{vn::math::deg_to_rad( 20.0f), Eigen::Vector3f{0.0f, 1.0f, 0.0f}}
                                      }.matrix();                                               

                //Eigen::Matrix4f rot = Eigen::Affine3f{ Eigen::AngleAxisf{vn::math::deg_to_rad( -25.0f), Eigen::Vector3f{1.0f, 0.0f, 0.0f}} }.matrix();

                Eigen::Matrix4f m = (physics_camera_.matrix() * cam_transform_).transpose();

                draw_task_queue_.consume([](auto&& f) { f(); }); // execute our waiting tasks..

                swirl_effect_.draw(ps..., current_id_, m);                                
                box2d_.draw(ps..., m);

                //physics_task_queue_.insert(gtl::physics::generators::polymorphic_generator{std::make_unique<render_box2d>(draw_kit_)});                            
            });
        }

        void resize(int w, int h, gtl::d3d::command_queue& c)
        {
            swirl_effect_.resize(w, h, c);
        }

        template <typename YieldType>
        gtl::event resizing_event_context(YieldType& yield) const
        {
            // timer begins
            // yes is pressed, no is pressed, esc is pressed, timer expires
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;

            while (!same_type(yield().get(), ev::exit_immediately{}))
            {
                if (same_type(yield.get(), ev::keydown{}))
                {
                    switch (boost::get<ev::keydown>(yield.get()).key)
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
            using k = gtl::keyboard;
            using namespace boost::units;
            using namespace gtl::physics::commands;

            int count{};

            //std::vector<gtl::physics::command_variant> task_local_;

            uint16_t selected_id{};
            bool shift_down_{};

            bool quit_{};
            // gtl::events::event_variant quit_event_{ev::none{}}

            auto mouse_handler = vn::make_lambda_visitor(
                [&](ev::mouse_wheel_scroll const& e) {
                    std::cout << "mouse scroll : new delta = " << e.wheel_delta << ", keystate == " << e.key_state << "\n";
                    camera_height_ += (e.wheel_delta > 0 ? -0.1f : 0.1f) * boost::units::si::meter;
                    std::cout << "camera's new height == " << camera_height_ / boost::units::si::meter << "\n";
                },
                [&](ev::mouse_lbutton_down const& e) {
                    selected_id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "selected object " << selected_id << "\n";

                    // int mx = GET_X_LPARAM(e.coord); // TODO replace this coordinate stuff..
                    // int my = GET_Y_LPARAM(e.coord);

                    // draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_down(e.x, e.y); });
                    // resource_callback_(gtl::commands::get_audio_adapter{}, [](auto&& aud) { aud.play_effect("click");
                    // }); // TODO causes an ICE..
                },
                [&](ev::mouse_rbutton_down const& e) {

                    selected_id = current_id_.load(std::memory_order_relaxed);

                    if (shift_down_)
                    {
                        physics_task_queue_.insert(destroy_object_implode{selected_id});
                        //physics_task_queue_.swap_in(task_local_); // TODO fix how tasks are submitted..
                    }
                    else
                    {
                        physics_task_queue_.insert(boost_object{selected_id});
                        //physics_task_queue_.swap_in(task_local_);
                    }
                },
                [&](ev::mouse_lbutton_up const& e) {
                    // int mx = GET_X_LPARAM(e.coord);
                    // int my = GET_Y_LPARAM(e.coord);
                    // draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_up(e.x, e.y); });
                },
                [&](ev::mouse_moved const& e) {
                    swirl_effect_.set_mouse_coords(e.x, e.y);

                    // int mx = GET_X_LPARAM(e.coord);
                    // int my = GET_Y_LPARAM(e.coord);
                    // draw_task_queue_.insert([=, this]() { this->imgui_adapter_.mouse_move(e.x, e.y); });
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
                            std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";
                            quit_handler(ev::exit_state{0});
                        }
                        break;
                        case k::A:
                        {
                            std::cout << "swirl_effect(): A pressed, generating new object.. \n";
                            //physics_task_queue_.insert();
                            physics_task_queue_.insert(dynamic_rectangle{
                                {0.0f * si::meter, 0.0f * si::meter}, {0.5f * si::meter, 0.5f * si::meter}, 0.0f * si::radians, gtl::entity::render_data{}.pack_entity_id(888)});
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
                    //physics_task_queue_.insert(boost_object_vec{selected_id, e.x / 4.0f, e.y / 4.0f}); // HACK
                    physics_task_queue_.insert(boost_object_vec{selected_id, e.x / 10.0f, e.y / 10.0f}); // HACK
                    //std::cout << "dpad..\n"; //physics_task_queue_.swap_in(task_local_);
                },
                [&](ev::resize_swapchain const&) {}, [](auto const&) {});

            while (!quit_)
            {                                                               
                top_level_handler(yield().get());
            }

            return ev::exit_state{0};
        }
    };
}
} // namespaces
#endif
