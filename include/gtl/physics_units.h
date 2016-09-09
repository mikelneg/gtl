#ifndef URUWOZOBBASFFW_GTL_PHYSICS_UNITS_H_
#define URUWOZOBBASFFW_GTL_PHYSICS_UNITS_H_

/*----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              

    namespace gtl::
    units for physics 

-----------------------------------------------------------------------------*/

#include <Eigen/Core>

#include <boost/units/base_units/angle/radian.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/plane_angle.hpp>

namespace gtl {
namespace physics {

    //template <typename T>
    //using area = boost::units::quantity<boost::units::si::area,T>;

    template <typename T>
    using length = boost::units::quantity<boost::units::si::length, T>;

    template <typename T>
    using dimensions = std::pair<length<T>, length<T> >;

    template <typename T>
    using position = std::pair<length<T>, length<T> >;

    template <typename T>
    using angle = boost::units::quantity<boost::units::angle::radian_base_unit::unit_type, T>;

} // namespace
} // namespace

#endif
