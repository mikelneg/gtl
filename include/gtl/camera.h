#ifndef IWOGZBOWOAFAFG_GTL_CAMERA_H_
#define IWOGZBOWOAFAFG_GTL_CAMERA_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    class camera
-----------------------------------------------------------------------------*/

#include <Eigen/Core>
#include <gtl/physics_units.h>
                     
namespace gtl {    

    class camera {         

        Eigen::Matrix4f transform_;
                        

    public:

        camera(gtl::physics::position<float> center_, 
               gtl::physics::dimensions<float> lens_dimensions_,
               gtl::physics::angle<float> fov_,
               gtl::physics::length<float> distance_to_lens_,
               gtl::physics::length<float> distance_to_plane_);          

        Eigen::Matrix4f const& matrix() const {
            return transform_;
        }

    };

} // namespaces
#endif