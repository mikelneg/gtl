#include "gtl/gui_rect_draw.h"

#include <array>
#include <algorithm>
#include <iostream>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <vn/math_utilities.h>

namespace gtl {
namespace d3d {



//void rect_draw::construct_vertices(std::vector<Eigen::Vector4f> const& v_centers_) const
//{        
//    
//    //std::vector<Vertex> v_centers_;
//    
//    // construct gui_dummy stuff..            
//    //for (unsigned i = 0; i < 20; ++i) {
//    //    v_centers_.emplace_back(Vertex{
//    //                            {vn::math::rand_neg_one_one(),vn::math::rand_neg_one_one(),0.1f,0.2f}, // xywh
//    //                            {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f} // uv in palette..                                        
//    //                           });
//    //}
//    
//    auto clamp = [](float v, float l = -1.0f, float u = 1.0f) {  if (v < l) { return l; } if (v > u) { return u; } return v; };    
//
//    auto insert_rect_ = 
//        [this,clamp](Eigen::Vector4f const& center_)
//        { 
//            Eigen::Vector4f const& pos = center_;
//            //Eigen::Vector4f& uv = center_.uv;
//            
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() - pos.z()),
//                                        clamp(pos.y() - pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() - pos.z()),
//                                        clamp(pos.y() - pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() - pos.z()),
//                                        clamp(pos.y() + pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() + pos.z()),
//                                        clamp(pos.y() - pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() + pos.z()),
//                                        clamp(pos.y() + pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//
//            mesh_.emplace_back(Vertex{ {clamp(pos.x() + pos.z()),
//                                        clamp(pos.y() + pos.w()), 1.0f, 1.0f}, 
//                                        {vn::math::rand_zero_one(),vn::math::rand_zero_one(), 0.0f, 0.0f}
//                              });
//        };
//
//    mesh_.clear();
//
//    Eigen::Vector4f dummy{0.0f,0.0f,0.0f,0.0f};
//    insert_rect_(dummy);
//
//    for (auto&& e : v_centers_) {
//        insert_rect_(e);        
//    }                   
//
//    insert_rect_(dummy);    
//}
//

}} // namespace
