#ifndef BHWOAOIJAVVASFAFW_GTL_D3D_GUI_TYPES_H_
#define BHWOAOIJAVVASFAFW_GTL_D3D_GUI_TYPES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl
    d3d gui types.. to be replaced
-----------------------------------------------------------------------------*/

#include <Eigen/Geometry>
#include <Eigen/StdVector>

namespace gtl {
namespace d3d {
namespace dummy_types {

    struct rect {        
        Eigen::Vector4f pos; // xyzw
        Eigen::Vector2f dims;  // wh
        uint32_t id; // identity..            
    };    

}}}
#endif
