/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/camera.h"

#include <Eigen/Geometry>
#include <gtl/physics/common_types.h>

namespace gtl {

namespace {

    Eigen::Matrix4f make_ortho_proj_matrix(float n, float f)
    {
        //        constexpr float l = -10.0f;
        //        constexpr float r = 10.0f;
        //
        //        constexpr float t = 10.0f;
        //        constexpr float b = -10.0f;

        float l = -1.0f;
        float r = 1.0f;

        float t = 1.0f;
        float b = -1.0f;

        n = -1.0f;
        f = 1.0f;
        // n = -1.0f;   // HACK fix this, currently set externally (but meaningless..)
        // f = 1.0f;

        // // D3DXMatrixOrthoLH()
        //   2/w  0    0           0
        //   0    2/h  0           0
        //   0    0    1/(zf-zn)   0
        //   0    0    zn/(zn-zf)  1

        Eigen::Matrix4f matrix_;
        
        // clang-format off
        
        matrix_ << 2.0f / (r - l), 0.0f, 0.0f, 0.0f, 
                   0.0f, 2.0f / (t - b), 0.0f, 0.0f, 
                   0.0f, 0.0f, 1.0f / (f - n), 0.0f,
                   0.0f, 0.0f, -n / (f - n), 1.0f;
        
        // clang-format on

        // matrix_ << 2.0f/(r-l), 0.0f, 0.0f, 0.0f,
        //           0.0f, 2.0f/(t-b), 0.0f, 0.0f,
        //           0.0f, 0.0f, 1.0f/(f-n), 0.0f,
        //           0.0f, 0.0f, 0.0f, 1.0f;
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

        matrix_ << s / aspect_ratio, 0.0f, 0.0f, 0.0f, 0.0f, s, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (z_far - z_near), 1.0f,
            0.0f, 0.0f, -z_near / (z_far - z_near), 0.0f;
        return matrix_;
    }

} // namespace

camera::camera(gtl::physics::position<float> center_, gtl::physics::dimensions<float> lens_dimensions_,
               gtl::physics::angle<float> fov_, gtl::physics::length<float> distance_to_lens_,
               gtl::physics::length<float> distance_to_plane_)
    : transform_{make_ortho_proj_matrix(distance_to_lens_ / boost::units::si::meters,
                                        distance_to_plane_ / boost::units::si::meters)}
// transform_{make_projection_matrix(fov_ / boost::units::si::radians,
//                                  lens_dimensions_.first / lens_dimensions_.second,
//                                  distance_to_lens_ / boost::units::si::meters,
//                                  distance_to_plane_ / boost::units::si::meters)}

//* Eigen::Affine3f{Eigen::Translation3f{center_.first / boost::units::si::meters,
//                                       center_.second / boost::units::si::meters,
//                                       distance_to_plane_ / boost::units::si::meters}}.matrix()}
{
}

} // namespace