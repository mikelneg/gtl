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

namespace gtl {
namespace scenes {

    class main_scene {

        gtl::swap_vector<gtl::physics::generator> mutable task_queue_;
        gtl::physics_simulation physics_;
        gtl::scenes::transitions::swirl_effect swirl_effect_; 

        std::atomic<uint32_t> mutable current_id_{};

    public:        
                
        main_scene(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue) 
            :   task_queue_{[](){
                    using namespace gtl::physics::generators;    
                    using namespace boost::units;
                    std::vector<gtl::physics::generator> generators_;
              
                    generators_.emplace_back(static_box{{0.0f * si::meters, -100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, 0});
                    generators_.emplace_back(static_box{ {0.0f * si::meters, 100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, 0});                
                    generators_.emplace_back(static_box{{-100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, 0});
                    generators_.emplace_back(static_box{ {100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, 0});
                    
                    for (unsigned i = 0; i < 300; ++i) {                            
                        generators_.emplace_back(dynamic_box{{vn::math::rand_neg_one_one() * 45 * si::meter,
                                                              vn::math::rand_neg_one_one() * 45 * si::meter},
                                                              {0.2f * si::meter, 
                                                               0.2f * si::meter}, vn::math::rand_neg_one_one() * si::radians, i + 400 });
                    }
              
                    return generators_; 
                }()}, // don't forget to call the lambda.. 
            physics_{task_queue_},
            swirl_effect_{dev,swchain,cqueue,physics_}
        {}

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const {
            func([&](auto&&...ps){ swirl_effect_.draw(std::forward<decltype(ps)>(ps)..., current_id_); });            
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
                                                                         {0.5f * si::meter, 0.5f * si::meter}, 0.0f * si::radians, 888});
                                    task_queue_.swap_in(task_local_);
                                    break;                        
                        
                        case k::K : std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";                                                
                                    throw std::runtime_error{__func__}; break;                    
                        
                        case k::R : std::cout << "swirl_effect() : r pressed, the id is -- " << current_id_.load(std::memory_order_relaxed) << "\n"; 

                                ;; 
                                // TODO NEXT THING TO ADD: screen to world coordinate transformations so that I can, e.g., 
                                                   nuke all objects within an area where I click (not simply on an object)

                                // TODO NEXT THING TO ADD AFTER THAT: timers, so I can click and have it implode n seconds later..
                        


                                    //resource_callback_(gtl::commands::get_swap_chain{},
                                    //                   [](auto& swchain_){ swchain_.resize(100,100); });                                                                        
                                    //resource_callback_(gtl::commands::get_some_resource{},[](auto& r) { r(); });
                                    break;
                        default : std::cout << "swirl_effect() : unknown key pressed\n"; 
                    }
                                   
                } else if (same_type(yield.get(),ev::mouse_lbutton_down{})) {
                    uint32_t id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "nuking object " << id << "\n";                    
                    task_local_.emplace_back(destroy_object_implode{id});
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
