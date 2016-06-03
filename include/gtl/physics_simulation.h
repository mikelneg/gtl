#ifndef RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_
#define RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_H_

/*----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl::
    class physics_simulation

-----------------------------------------------------------------------------*/

#include <vector>
#include <gtl/swap_vector.h>
#include <thread>

#include <atomic>
#include <Eigen/Core>

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/base_units/angle/radian.hpp>

#include <boost/variant.hpp>

namespace gtl {
    namespace physics {

        //template <typename T>    
        //using area = boost::units::quantity<boost::units::si::area,T>;              

        template <typename T>
        using length = boost::units::quantity<boost::units::si::length,T>;

        template <typename T>
        using dimensions = std::pair<length<T>,length<T>>;

        template <typename T>
        using position = std::pair<length<T>,length<T>>;
        
        template <typename T>
        using angle = boost::units::quantity<boost::units::angle::radian_base_unit::unit_type,T>;

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

            struct static_circle {
                float x,y,r,a;
                uint32_t id;                
            };


            struct destroy_object_implode {
                uint32_t id;
            };


        }

        using generator = boost::variant<generators::static_box,
                                         generators::dynamic_box,
                                         generators::static_circle,
                                         generators::destroy_object_implode>;
    }

    struct EntityInfo {
        Eigen::Vector4f xywh_;
        Eigen::Vector4f rgb_angle_;
        uint32_t id_;
    };

    
    class physics_simulation {        
        using entity_type = EntityInfo;

        //gtl::swap_vector<> entities_;
        gtl::swap_vector<entity_type> entities_;
                
        std::atomic_flag quit_;
        std::thread thread_;

    public:
   
        physics_simulation(gtl::swap_vector<gtl::physics::generator>&);

        bool extract_positions(std::vector<entity_type>& c) { return entities_.swap_out(c); }

        std::vector<entity_type> copy_out() { return entities_.copy_out(); }

        ~physics_simulation() {
            quit_.clear();
            thread_.join();
        }
    };

} // namespace
#endif