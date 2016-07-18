#include "gtl/camera.h"

#include <gtl/physics_units.h>
#include <Eigen/Geometry>

namespace gtl {

    namespace {
        
    Eigen::Matrix4f make_ortho_proj_matrix(float n, float f) {
        constexpr float l = -100.0f;
        constexpr float r = 100.0f;

        constexpr float t = 100.0f;
        constexpr float b = -100.0f;

        n = 0.0f;   // HACK fix this, currently set externally (but meaningless..)
        f = 200.0f;

        // // D3DXMatrixOrthoLH()
        //   2/w  0    0           0
        //   0    2/h  0           0
        //   0    0    1/(zf-zn)   0
        //   0    0    zn/(zn-zf)  1


        Eigen::Matrix4f matrix_;
        matrix_ << 2.0f/(r-l), 0.0f, 0.0f, 0.0f,
                   0.0f, 2.0f/(t-b), 0.0f, 0.0f,
                   0.0f, 0.0f, 1.0f/(f-n), 0.0f,
                   0.0f, 0.0f, n/(n-f),    1.0f;
            //(l+r)/(l-r), (t+b)/(b-t), n/(n-f), 1.0f; //-(r+l)/(r-l),-(t+b)/(t-b),-(f+n)/(f-n), 1.0f;        
        return matrix_;
        }
    

        Eigen::Matrix4f make_projection_matrix(float fov_y, float aspect_ratio, float z_near, float z_far) 
        {            
            float s = 1.0f / std::tan(fov_y * 0.5f);
            Eigen::Matrix4f matrix_;
        
            z_near = z_near / z_far;
            z_far = 1.0f;

            //  f(z,near,far) = [ z * (1/(far-near)) + (-near/(far-near)) ] / z
            //  maps z between near and far to [0...1]
            //  recommended that you graph this with your near/far values to examine
            //  the scale
        
            matrix_ << s/aspect_ratio, 0.0f, 0.0f, 0.0f,
                       0.0f, s, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f/(z_far-z_near), 1.0f,
                       0.0f, 0.0f, -z_near/(z_far-z_near), 0.0f;
            return matrix_;
        }

    } // namespace
    

    camera::camera(gtl::physics::position<float> center_, 
                   gtl::physics::dimensions<float> lens_dimensions_,
                   gtl::physics::angle<float> fov_,
                   gtl::physics::length<float> distance_to_lens_,
                   gtl::physics::length<float> distance_to_plane_)
        :   transform_{make_ortho_proj_matrix(distance_to_lens_ / boost::units::si::meters, 
                                              distance_to_plane_ / boost::units::si::meters)}
            //transform_{make_projection_matrix(fov_ / boost::units::si::radians, 
            //                                  lens_dimensions_.first / lens_dimensions_.second,
            //                                  distance_to_lens_ / boost::units::si::meters, 
            //                                  distance_to_plane_ / boost::units::si::meters)}
            
                        //* Eigen::Affine3f{Eigen::Translation3f{center_.first / boost::units::si::meters,
                        //                                       center_.second / boost::units::si::meters,
                        //                                       distance_to_plane_ / boost::units::si::meters}}.matrix()}                                
    {}


} // namespace