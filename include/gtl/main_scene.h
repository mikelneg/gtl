#ifndef LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_
#define LKJLKJLAIEJFASS_GTL_SCENES_MAIN_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scenes
    
    class main_scene;
-----------------------------------------------------------------------------*/

#include <gtl/events.h>
#include <gtl/keyboard_enum.h>
#include <gtl/d3d_types.h>

#include <gtl/swirl_effect_transition_scene.h>
#include <gtl/command_variant.h>

#include <gtl/physics_simulation.h>
#include <gtl/swap_vector.h>

#include <vn/math_utilities.h>

#include <Eigen/Core>
#include <atomic>
#include <cmath>

#include <gtl/camera.h>

namespace gtl {
namespace scenes {

    class main_scene {

        gtl::swap_vector<gtl::physics::generator> mutable task_queue_;
        gtl::physics_simulation physics_;
        gtl::camera physics_camera_;
        
        gtl::physics::length<float> mutable camera_height_;            

        gtl::scenes::transitions::swirl_effect swirl_effect_;         

        std::atomic<uint32_t> mutable current_id_{};

    public:        
                
        main_scene(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue) 
            :   task_queue_{[](){
                    using namespace gtl::physics::generators;    
                    using namespace boost::units;
                    std::vector<gtl::physics::generator> generators_;
              
                    generators_.emplace_back(static_box{{0.0f * si::meters, -100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
                    generators_.emplace_back(static_box{ {0.0f * si::meters, 100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});                
                    generators_.emplace_back(static_box{{-100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
                    generators_.emplace_back(static_box{ {100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
                    
                    //for (unsigned i = 0; i < 100; ++i) {                            
                    //    generators_.emplace_back(dynamic_box{{vn::math::rand_neg_one_one() * 45 * si::meter,
                    //                                          vn::math::rand_neg_one_one() * 45 * si::meter},
                    //                                          {1.0f * si::meter, 
                    //                                           1.0f * si::meter}, vn::math::rand_neg_one_one() * si::radians, i + 200 });
                    //}

                    for (unsigned j = 0; j < 80; ++j) { 
                        std::vector<dynamic_box> jointed_boxes_;
                        auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto angle = 0.0f * si::radians; //vn::math::rand_neg_one_one() * si::radians;

                        for (unsigned i = 0; i < 4; ++i) {                            
                            jointed_boxes_.emplace_back(dynamic_box{{x,y - ((i * 1.0f) * si::meter)},
                                                                    {1.0f * si::meter, 
                                                                     1.0f * si::meter}, angle, 
                                                                    // HACK computes mesh+entity id 
                                               InstanceInfo{}.pack_entity_id(j + 600)
                                                                 .pack_mesh_id(0)});
                        }              
                        generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_)});                        
                    }

                    for (unsigned j = 80; j < 160; ++j) { 
                        std::vector<dynamic_box> jointed_boxes_;
                        auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto angle = 0.0f * si::radians; //vn::math::rand_neg_one_one() * si::radians;

                        //for (unsigned i = 0; i < 4; ++i) {                            
                          //  jointed_boxes_.emplace_back(
                             generators_.emplace_back(dynamic_box{{x,y},
                                                                    {1.0f * si::meter, 
                                                                     1.0f * si::meter}, angle,                                                                     
                                                   InstanceInfo{}.pack_entity_id(j + 600)
                                                                 .pack_mesh_id(1)});
                        //}              
                        //generators_.emplace_back(dynamic_jointed_boxes{std::move(jointed_boxes_)});                        
                    }


                    return generators_; 
                }()}, // don't forget to call the lambda.. 
            physics_camera_{{0.0f * boost::units::si::meters, 0.0f * boost::units::si::meters},
                            {1.0f * boost::units::si::meters, 1.0f * boost::units::si::meters},
                            gtl::physics::angle<float>{45.0f * boost::units::degree::degree},
                            {0.001f * boost::units::si::meters},
                            {100.0f * boost::units::si::meters}},
            camera_height_{100.0f * boost::units::si::meters},
            physics_{task_queue_},
            swirl_effect_{dev,swchain,cqueue,physics_}            
        {}

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const {
            func([&](auto&&...ps){                 
                swirl_effect_.draw(std::forward<decltype(ps)>(ps)..., 
                                   current_id_, 
                                   physics_camera_.matrix() * Eigen::Affine3f{Eigen::Translation3f{0.0f,0.0f,camera_height_ / boost::units::si::meters}}.matrix());
            });
        }
                
        template <typename ResourceManager, typename YieldType>
        gtl::event handle_events(ResourceManager& resource_callback_, YieldType& yield) const {
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;
            using namespace boost::units;
            using namespace gtl::physics::generators;

            int count{};

            std::vector<gtl::physics::generator> task_local_;

            std::cout << "swirl_effect event handler entered..\n"; 
            while (!same_type(yield().get(),ev::exit_immediately{})){                   
                if (same_type(yield.get(),ev::keydown{})){                     
                    switch( boost::get<ev::keydown>( yield.get().value() ).key ) {
                        case k::Escape : std::cout << "swirl_effect(): escape pressed, exiting all..\n"; 
                                         return gtl::events::exit_all{}; break;
                        case k::Q : std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
                                    return gtl::events::exit_state{0}; break;
                        
                        case k::A : std::cout << "swirl_effect(): A pressed, generating new object.. \n";                             
                                    task_local_.emplace_back(dynamic_box{{0.0f * si::meter, 0.0f * si::meter}, 
                                                                         {0.5f * si::meter, 0.5f * si::meter}, 0.0f * si::radians, 
                                                                InstanceInfo{}.pack_entity_id(888)});
                                    task_queue_.swap_in(task_local_);
                                    break;                        
                        
                        case k::K : std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";                                                
                                    throw std::runtime_error{__func__}; break;                    
                        
                        case k::R : std::cout << "swirl_effect() : r pressed, the id is -- " << current_id_.load(std::memory_order_relaxed) << "\n"; 

                                // TODO add screen to world coordinate transformations so that I can, e.g., 
                                //                 nuke all objects within an area where I click (not simply on an object)

                                // TODO add timers, so I can click and have it implode n seconds later..
                                // TODO add id system to renderer: (pos,angle,index) are returned from physics, the rest is stored on the
                                //          other side
                        
                                    //resource_callback_(gtl::commands::get_swap_chain{},
                                    //                   [](auto& swchain_){ swchain_.resize(100,100); });                                                                        
                                    //resource_callback_(gtl::commands::get_some_resource{},[](auto& r) { r(); });
                                    break;
                        default : std::cout << "swirl_effect() : unknown key pressed\n"; 
                    }
                                   
                } else if (same_type(yield.get(),ev::mouse_wheel_scroll{})) {
                    //uint32_t id = current_id_.load(std::memory_order_relaxed);
                    gtl::events::mouse_wheel_scroll const& event_ = boost::get<ev::mouse_wheel_scroll>(yield.get().value());
                    std::cout << "mouse scroll : new delta = " << event_.wheel_delta << ", keystate == " << event_.key_state << "\n";                                        
                    camera_height_ += (event_.wheel_delta > 0 ? -1.0f : 1.0f) * boost::units::si::meter;
                    std::cout << "camera's new height == " << camera_height_ / boost::units::si::meter << "\n";                    
                    //task_local_.emplace_back(destroy_object_implode{id});
                    //task_queue_.swap_in(task_local_);
                } else if (same_type(yield.get(),ev::mouse_lbutton_down{})) {
                    uint16_t id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "nuking object " << id << "\n";                    
                    task_local_.emplace_back(destroy_object_implode{id});
                    task_queue_.swap_in(task_local_);

                } else if (same_type(yield.get(),ev::mouse_rbutton_down{})) {
                    uint16_t id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "boosting object " << id << "\n";                    
                    task_local_.emplace_back(boost_object{id});
                    task_queue_.swap_in(task_local_);

                } else if (same_type(yield.get(),ev::none{})) {
                    count++;                
                } else if (same_type(yield.get(),ev::mouse_at{})) {
                    swirl_effect_.set_mouse_coords(boost::get<ev::mouse_at>(yield.get().value()).coord);                    
                }
            }            
            return gtl::events::exit_state{0};
        }
    };

}} // namespaces
#endif
