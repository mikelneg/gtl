#ifndef RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_
#define RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_

/*----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl::
    class physics_simulation

-----------------------------------------------------------------------------*/

#include <vector>
#include <gtl/swap_vector.h>
#include <vn/swap_object.h>

#include <thread>

#include <atomic>
#include <Eigen/Core>

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/base_units/angle/radian.hpp>

#include <boost/variant.hpp>

#include <gtl/physics_units.h>

namespace gtl {
    namespace physics {

        namespace generators {                                                        
        
            struct static_box {                                                                
                position<float> xy_;
                dimensions<float> wh_;
                angle<float> angle_;                                               
                uint32_t id;                
            };

            struct dynamic_box {                                                                                
                position<float> xy_;
                dimensions<float> wh_;
                angle<float> angle_;                
                uint32_t id;                
            };

            struct dynamic_jointed_boxes {                                                                                                
                std::vector<dynamic_box> boxes_;
                uint32_t id;                
            };

            struct static_circle {
                float x,y,r,a;
                uint32_t id;                
            };


            struct destroy_object_implode {
                uint32_t id;
            };

            struct boost_object {
                uint32_t id;
            };


        }

        using generator = boost::variant<generators::static_box,
                                         generators::dynamic_box,
                                         generators::dynamic_jointed_boxes,
                                         generators::static_circle,
                                         generators::destroy_object_implode,
                                         generators::boost_object>;
    }

    struct EntityInfo {
        //Eigen::Vector4f xywh_;
        //Eigen::Vector4f rgb_angle_;                
        uint32_t bone_offset_;
        uint32_t bone_count_;

        uintptr_t entity_data_; 
        // EntityData... 
        // uint32_t mesh_id_;
        // uint32_t id_;
        // etc..
    };

    static_assert(sizeof(EntityInfo) == sizeof(uintptr_t) * 2, "EntityInfo not packed properly..");

    
    struct render_data {
        std::vector<EntityInfo> entities_;
        std::vector<Eigen::Matrix4f> bones_;

        friend void swap(render_data& lhs, render_data& rhs) {
            using std::swap;
            swap(lhs.entities_, rhs.entities_);
            swap(lhs.bones_, rhs.bones_);
        }
    };

    class physics_simulation {        
        using entity_type = EntityInfo;

        //gtl::swap_vector<> entities_;
        //gtl::swap_vector<entity_type> entities_;

        vn::swap_object<render_data> render_data_;
                
        std::atomic_flag quit_;
        std::thread thread_;

    public:
   
        physics_simulation(gtl::swap_vector<gtl::physics::generator>&);        

        bool extract_render_data(render_data& c) { return render_data_.swap_out(c); }
        //bool extract_positions(std::vector<entity_type>& c) { return entities_.swap_out(c); }

        // std::vector<entity_type> copy_out() { return rend.copy_out(); }

        ~physics_simulation() {
            quit_.clear();
            thread_.join();
        }
    };

} // namespace
#endif