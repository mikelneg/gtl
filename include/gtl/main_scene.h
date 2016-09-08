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
#include <gtl/d3d_imgui_adapter.h>
#include <gtl/command_variant.h>

#include <gtl/physics_simulation.h>
#include <gtl/swap_vector.h>
#include <vn/swap_object.h>

//#include <gtl/event_listener.h>

#include <vn/math_utilities.h>
#include <vn/single_consumer_queue.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <atomic>
#include <functional>
#include <cmath>
#include <vector>

#include <gtl/camera.h>

#include <GamePad.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>


namespace gtl {
namespace scenes {

    class main_scene { //: public gtl::event_listener<main_scene,gtl::command_variant> {

        gtl::swap_vector<gtl::physics::generator> mutable physics_task_queue_;  // TODO consider using vn::single_consumer_queue
        gtl::physics_simulation physics_;
        gtl::camera physics_camera_;
        
        gtl::physics::length<float> mutable camera_height_;            

        gtl::scenes::transitions::swirl_effect swirl_effect_;
        
        gtl::imgui_adapter mutable imgui_adapter_;
        gtl::d3d::imgui_adapter imgui_;

        std::atomic<uint32_t> mutable current_id_{};
        
        std::atomic<int> mutable focus_id_{};

        vn::single_consumer_queue<std::function<void()>> mutable draw_task_queue_;


        //std::vector<char> mutable text_box_, other_box_;
        

    public:        
                
        main_scene(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain, gtl::d3d::command_queue& cqueue) 
            :   physics_task_queue_{[](){
                    using namespace gtl::physics::generators;    
                    using namespace boost::units;
                    std::vector<gtl::physics::generator> generators_;
              
                    generators_.emplace_back(static_box{{0.0f * si::meters, -100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});
                    generators_.emplace_back(static_box{ {0.0f * si::meters, 100.0f * si::meters},{200.0f * si::meters, 5.0f * si::meters}, 0.0f * si::radians, {}});                
                    generators_.emplace_back(static_box{{-100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
                    generators_.emplace_back(static_box{ {100.0f * si::meters, 0.0f * si::meters},{5.0f * si::meters, 200.0f * si::meters}, 0.0f * si::radians, {}});
                                       
                    for (unsigned j = 0; j < 80; ++j) { 
                        std::vector<dynamic_box> jointed_boxes_;
                        auto x = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto y = vn::math::rand_neg_one_one() * 45.0f * si::meter;
                        auto angle = 0.0f * si::radians; //vn::math::rand_neg_one_one() * si::radians;

                        for (unsigned i = 0; i < 4; ++i) {                            
                            jointed_boxes_.emplace_back(dynamic_box{{x,y - ((i * 1.0f) * si::meter)},
                                                                    {1.0f * si::meter, 
                                                                     1.0f * si::meter}, angle,                                                                     
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
                }()}, 
            physics_camera_{{0.0f * boost::units::si::meters, 0.0f * boost::units::si::meters},
                            {1.0f * boost::units::si::meters, 1.0f * boost::units::si::meters},
                            gtl::physics::angle<float>{45.0f * boost::units::degree::degree},
                            {0.001f * boost::units::si::meters},
                            {100.0f * boost::units::si::meters}},
            camera_height_{10.0f * boost::units::si::meters},
            physics_{physics_task_queue_},
            swirl_effect_{dev,swchain,cqueue,physics_},
            imgui_adapter_{},
            imgui_{dev, swchain, cqueue, imgui_adapter_}
        {
            //imgui_.insert_callback("steal_focus", [&](){ focus_id_.store(1,std::memory_order_release); });
            //imgui_.insert_callback("return_focus", [&](){ focus_id_.store(0,std::memory_order_release); });
            //focus_id_.store(0,std::memory_order_release);

            //std::string s{"hi there"};
            //text_box_.insert(end(text_box_),begin(s),end(s));
            //other_box_.insert(end(other_box_),begin(s),end(s));
            //text_box_.resize(256);
            //other_box_.resize(256);
            imgui_adapter_.render();
        }

        main_scene(main_scene&&) = default;

        template <typename F>
        void draw_callback(F func) const {
            func([&](auto&&...ps){                 
                Eigen::Matrix4f cam_transform_ = Eigen::Affine3f{Eigen::Scaling(1.0f / (camera_height_ / boost::units::si::meter)) //}.matrix();                                                                 
                                                                 * Eigen::AngleAxisf{0.2f, Eigen::Vector3f{0.0f,0.0f,-1.0f}}
                                                                 * Eigen::AngleAxisf{0.2f, Eigen::Vector3f{-1.0f,0.0f,0.0f}}}.matrix();           
                                                
                draw_task_queue_.consume([](auto&& f){ f(); });    // execute our waiting tasks..
                
                swirl_effect_.draw(ps...,current_id_, cam_transform_ * physics_camera_.matrix());                                    
                

                imgui_adapter_.render();
                imgui_.draw(ps...);                    
            });
        }
                        

        //void listen(gtl::command_variant) {
        //    std::cout << "main_scene got my message..\n"; 
        //}

        void resize(int w, int h, gtl::d3d::command_queue& c) {
            swirl_effect_.resize(w,h,c);
            imgui_.resize(w,h,c);
        }

        template <typename YieldType>
        gtl::event 
        resizing_event_context(YieldType& yield) const {
            // timer begins
            // yes is pressed, no is pressed, esc is pressed, timer expires
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;
            
            while (!same_type(yield().get(),ev::exit_immediately{})){                                   
                if (same_type(yield.get(),ev::keydown{})){                     
                    switch( boost::get<ev::keydown>( yield.get().value() ).key ) {
                        case k::Escape :    // falls through..
                        case k::N      :    return gtl::events::revert{};
                                            break;

                        case k::Y      :    return gtl::events::keep{}; 
                                            break;
                        default        :    break;
                    }
                }
            }
            return gtl::events::revert{};        
        }          

        template <typename ResourceManager, typename YieldType>
        gtl::event handle_events(ResourceManager& resource_callback_, YieldType& yield) const {
            namespace ev = gtl::events;
            namespace k = gtl::keyboard;
            using namespace boost::units;
            using namespace gtl::physics::generators;

            int count{};

            std::vector<gtl::physics::generator> task_local_;

            uint16_t selected_id{};

            std::cout << "swirl_effect event handler entered..\n";

            while (!same_type(yield().get(),ev::exit_immediately{})){                   
                
                if (same_type(yield.get(),ev::keydown{})){                     
                    auto f_id = focus_id_.load(std::memory_order_acquire);

                    if (f_id == 0) {
                        std::cout << "main focus keypress..\n";
                    switch( boost::get<ev::keydown>( yield.get().value() ).key ) {
                        case k::Escape : std::cout << "swirl_effect(): escape pressed, exiting all..\n"; 
                                         return gtl::events::exit_all{}; break;
                        case k::Q : std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
                                    return gtl::events::exit_state{0}; break;
                        
                        case k::A : std::cout << "swirl_effect(): A pressed, generating new object.. \n";                             
                                    task_local_.emplace_back(dynamic_box{{0.0f * si::meter, 0.0f * si::meter}, 
                                                                         {0.5f * si::meter, 0.5f * si::meter}, 0.0f * si::radians, 
                                                                InstanceInfo{}.pack_entity_id(888)});
                                    physics_task_queue_.swap_in(task_local_);
                                    break;                        
                        
                        case k::K : std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";                                                
                                    throw std::runtime_error{__func__}; break;                    
                        
                        case k::R : {
                                        std::cout << "swirl_effect() : r pressed, issuing resize..\n";

                                        std::pair<int,int> dims{};
                                        resource_callback_(gtl::commands::get_swap_chain{},[&](auto& s){ dims = s.dimensions(); });
                                        resource_callback_(gtl::commands::resize{1280,720},[](){});
                
                                        std::cout << "entering resize context..\n";                                    
                                        if (same_type(resizing_event_context(yield),ev::keep{})) {
                                            std::cout << "keeping new size..\n";
                                        } else {
                                            std::cout << "reverting to old size..\n";
                                            resource_callback_(gtl::commands::resize{dims.first,dims.second},[](){});
                                        }

                                    }
                                // TODO add timers, so I can click and have it implode n seconds later..
                                // TODO add id system to renderer: (pos,angle,index) are returned from physics, the rest is stored on the
                                //          other side
                        
                                    //resource_callback_(gtl::commands::get_swap_chain{}, // TODO implement global resizing..
                                    //                   [](auto& swchain_){ swchain_.resize(100,100); });                                                                        
                                    //resource_callback_(gtl::commands::get_some_resource{},[](auto& r) { r(); });
                                    break;
                        default : std::cout << "swirl_effect() : unknown key pressed\n"; 
                    }
                   } else if (f_id == 1){
                       std::cout << "imgui focus keypress..\n";
                       draw_task_queue_.insert([c=boost::get<ev::keydown>(yield.get().value()).key,this](){ this->imgui_adapter_.add_input_charcter(c); });                       
                   }
                } else if (same_type(yield.get(),ev::mouse_wheel_scroll{})) {
                    //uint32_t id = current_id_.load(std::memory_order_relaxed);
                    gtl::events::mouse_wheel_scroll const& event_ = boost::get<ev::mouse_wheel_scroll>(yield.get().value());
                    std::cout << "mouse scroll : new delta = " << event_.wheel_delta << ", keystate == " << event_.key_state << "\n";                                        
                    camera_height_ += (event_.wheel_delta > 0 ? -0.1f : 0.1f) * boost::units::si::meter;
                    std::cout << "camera's new height == " << camera_height_ / boost::units::si::meter << "\n";                    
                    //task_local_.emplace_back(destroy_object_implode{id});
                    //physics_task_queue_.swap_in(task_local_);
                } else if (same_type(yield.get(),ev::mouse_lbutton_down{})) {
                    selected_id = current_id_.load(std::memory_order_relaxed);
                    //std::cout << "nuking object " << id << "\n";                    
                    std::cout << "selected object " << selected_id << "\n"; 
                
                    auto& coord_ = boost::get<ev::mouse_lbutton_down>(yield.get().value()).coord; // HACK fix this 
                    int mx = GET_X_LPARAM(coord_);
                    int my = GET_Y_LPARAM(coord_);

                    draw_task_queue_.insert([=,this](){ this->imgui_adapter_.mouse_down(mx,my); });
                    resource_callback_(gtl::commands::get_audio_adapter{}, [](auto& aud) { aud.play_effect("click"); });
                    //task_local_.emplace_back(destroy_object_implode{id});
                    //physics_task_queue_.swap_in(task_local_);
                } else if (same_type(yield.get(),ev::mouse_lbutton_up{})) {
                    
                    auto& coord_ = boost::get<ev::mouse_lbutton_up>(yield.get().value()).coord; // HACK fix this 
                    int mx = GET_X_LPARAM(coord_);
                    int my = GET_Y_LPARAM(coord_);
                    draw_task_queue_.insert([=,this](){ this->imgui_adapter_.mouse_up(mx,my); });
                    
                } else if (same_type(yield.get(),ev::mouse_rbutton_down{})) {
                    uint16_t id = current_id_.load(std::memory_order_relaxed);
                    std::cout << "boosting object " << id << "\n";                    
                    task_local_.emplace_back(boost_object{id});
                    physics_task_queue_.swap_in(task_local_);
                } else if (same_type(yield.get(),ev::dpad_pressed{})) {       
                    auto& dpad_press = boost::get<ev::dpad_pressed>(yield.get().value());
                    //std::cout << "dpad pressed.." << dpad_press.x << "," << dpad_press.y << "\n";
                    //uint16_t id = current_id_.load(std::memory_order_relaxed);                  
                    task_local_.emplace_back(boost_object_vec{selected_id,dpad_press.x / 400.0f,dpad_press.y / 400.0f});
                    physics_task_queue_.swap_in(task_local_);
                } else if (same_type(yield.get(),ev::none{})) {
                    count++;                
                } else if (same_type(yield.get(),ev::mouse_moved{})) {
                    swirl_effect_.set_mouse_coords(boost::get<ev::mouse_moved>(yield.get().value()).coord);

                    auto& coord_ = boost::get<ev::mouse_moved>(yield.get().value()).coord; // HACK fix this 
                    int mx = GET_X_LPARAM(coord_);
                    int my = GET_Y_LPARAM(coord_);
                    draw_task_queue_.insert([=,this](){ this->imgui_adapter_.mouse_move(mx,my); });
                } else if (same_type(yield.get(),ev::resize_swapchain{}) ) {
                    
                    
                    //int nw = yield.get().value().new_width;
                    //int nh = yield.get().value().new_height;
                    //
                    //resource_callback_(gtl::commands::get_swap_chain{},
                    //                   [=](auto& swchain){ swchain.resize(nw,nh); });
                    //
                    //

                }
            }            
            return gtl::events::exit_state{0};
        }
    };

}} // namespaces
#endif
