#include "../Include/font_atlas.h"


#include <string>
#include <array>
#include <gtl/include/font_atlas_glyph.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <iostream>

namespace gtl {
namespace d3d {

static 
std::array<Eigen::Vector4f,5> 
uv_for_character(font_atlas_glyph& f, uint8_t c)
{
    std::array<Eigen::Vector4f, 5> arr;  

    Glyph& g = f.glyph_map.at(c);

        //
        // arr[0] arr[1] == texture coordinates 
        // arr[2] arr[3] == dimensions for quad
    
    //arr[0] = Eigen::Vector4f{static_cast<float>(g.u + f.padding[0]), static_cast<float>(g.v + f.padding[1]), 
    //                       static_cast<float>(g.u + f.padding[0]), static_cast<float>(g.v + g.height - f.padding[3])};
    //arr[1] = Eigen::Vector4f{static_cast<float>(g.u + g.width - f.padding[2]), static_cast<float>(g.v + f.padding[1]), 
    //                       static_cast<float>(g.u + g.width - f.padding[2]), static_cast<float>(g.v + g.height - f.padding[3])};
    
    //arr[0] = Eigen::Vector4f{static_cast<float>(g.u - 1), static_cast<float>(g.v - 1),
    //                                          static_cast<float>(g.u - 1), static_cast<float>(g.v + g.height + 1)};
    //arr[1] = Eigen::Vector4f{static_cast<float>(g.u + g.width + 1), static_cast<float>(g.v - 1),
    //                                          static_cast<float>(g.u + g.width + 1), static_cast<float>(g.v + g.height + 1)};

    arr[0] = Eigen::Vector4f{g.u, g.v,
                           g.u, g.v + g.height};
    arr[1] = Eigen::Vector4f{g.u + g.width, g.v,
                           g.u + g.width, g.v + g.height};
    

    //arr[0] = Eigen::Vector4f{g.u - 3.0f, g.v - 3.0f,
    //                       g.u - 3.0f, g.v + g.height + 3.0f};
    //arr[1] = Eigen::Vector4f{g.u + g.width + 3.0f, g.v - 3.0f,
    //                       g.u + g.width + 3.0f, g.v + g.height + 3.0f};
    //

    //arr[0] = Eigen::Vector4f{static_cast<float>(g.u), static_cast<float>(g.v),
    //                       static_cast<float>(g.u), static_cast<float>(g.v + g.height)};
    //arr[1] = Eigen::Vector4f{static_cast<float>(g.u + g.width), static_cast<float>(g.v),
    //                       static_cast<float>(g.u + g.width), static_cast<float>(g.v + g.height)};


    //arr[2] = Eigen::Vector4f{static_cast<float>(g.x_offset), static_cast<float>(g.y_offset), 
    //                       static_cast<float>(g.x_offset), static_cast<float>(g.height)};
    //arr[3] = Eigen::Vector4f{static_cast<float>(g.x_advance - g.x_offset), static_cast<float>(g.y_offset), 
    //                       static_cast<float>(g.x_advance - g.x_offset), static_cast<float>(g.height)};
    
    //<char id = "73" x = "510" y = "218" width = "12" height = "88" xoffset = "11.81" yoffset = "6.69" xadvance = "35.56" page = "0" chnl = "15"/>

    
    arr[2] = Eigen::Vector4f{static_cast<float>(g.x_offset), static_cast<float>(g.y_offset), 
                           static_cast<float>(g.x_offset), static_cast<float>(g.y_offset + g.height)};
    arr[3] = Eigen::Vector4f{static_cast<float>(g.x_offset + g.width), static_cast<float>(g.y_offset), 
                           static_cast<float>(g.x_offset + g.width), static_cast<float>(g.y_offset + g.height)};
                          
    arr[4] = Eigen::Vector4f{g.x_advance,0.0f,0.0f,0.0f};

    //arr[0] = Eigen::Vector4f{static_cast<float>(g.u), static_cast<float>(g.v), static_cast<float>(g.u), static_cast<float>(g.v + g.height)};
    //arr[1] = Eigen::Vector4f{static_cast<float>(g.u + g.width), static_cast<float>(g.v), static_cast<float>(g.u + g.width), static_cast<float>(g.v + g.height)};
    //
    //arr[2] = Eigen::Vector4f{static_cast<float>(g.x_offset), static_cast<float>(g.y_offset), static_cast<float>(g.x_offset + g.x_advance), static_cast<float>(g.y_offset)};
    //arr[3] = Eigen::Vector4f{static_cast<float>(g.x_offset), static_cast<float>(g.height), static_cast<float>(g.x_offset + g.x_advance), static_cast<float>(g.height)};



    //arr[0] = Eigen::Vector4f{static_cast<float>(g.u + g.x_offset), static_cast<float>(g.v + g.y_offset), 0.0f, 0.0f};
    //arr[1] = Eigen::Vector4f{static_cast<float>(g.u + g.x_offset), static_cast<float>(g.v + g.y_offset + g.height), 0.0f, 0.0f};
    //arr[2] = Eigen::Vector4f{static_cast<float>(g.u + g.x_offset + g.x_advance), static_cast<float>(g.v + g.y_offset), 0.0f, 0.0f};
    //arr[3] = Eigen::Vector4f{static_cast<float>(g.u + g.x_offset + g.x_advance), static_cast<float>(g.v + g.y_offset + g.height), 0.0f, 0.0f};

    return arr;        
}

void font_atlas::set_message(std::string const& message) const
{
    mesh_.clear();
    
    float ratio = font_definition.base_width / font_definition.line_height;
    
    Eigen::Vector4f coord{0.0f, font_definition.base_width / (1280.0f * 3.0f), 1.0f, 1.0f};
    
    float xspan = coord.y();
    float lheight = xspan * (font_definition.line_height / font_definition.base_width);
    
    float lwidth = xspan;
         
   
    for (int i = 0; i < message.size(); ++i) {
        auto uv_array = uv_for_character(font_definition, message[i]);      

        if (i > 0) {
            auto it = font_definition.kerning_map.equal_range(message[i-1]);
            for (auto x = it.first; x != it.second; ++x) {
                if (x->second.first == message[i]) {
                    coord.x() += x->second.second * xspan;
                }
            }            
        }

        
        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[2].x() * lwidth + coord.x(),
                                               -uv_array[2].y() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[0].x(),uv_array[0].y(), 0.0f, 0.0f}});
        
        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[2].x() * lwidth + coord.x(),
                                               -uv_array[2].w() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[0].x(),uv_array[0].w(), 0.0f, 0.0f}});
        
        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[3].x() * lwidth + coord.x(),
                                               -uv_array[3].y() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[1].x(),uv_array[0].y(), 0.0f, 0.0f}});
        

        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[3].x() * lwidth + coord.x(),
                                              -uv_array[3].y() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[1].x(),uv_array[0].y(), 0.0f, 0.0f}});

        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[2].x() * lwidth + coord.x(),
                                              -uv_array[2].w() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[0].z(),uv_array[0].w(), 0.0f, 0.0f}});

        
        mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[3].x() * lwidth + coord.x(),
                                               -uv_array[2].w() * lheight, 1.0f, 1.0f},
                                               Eigen::Vector4f{uv_array[1].z(),uv_array[1].w(), 0.0f, 0.0f}});

        

        //mesh_.emplace_back(Vertex{Eigen::Vector4f{coord.x(),
        //                                       -uv_array[2].y() * lheight, 1.0f, 1.0f},
        //                                       Eigen::Vector4f{uv_array[0].x(),uv_array[0].y(), 0.0f, 0.0f}});
        //mesh_.emplace_back(Vertex{Eigen::Vector4f{coord.x(),
        //                                       -uv_array[2].y() * lheight, 1.0f, 1.0f},
        //                                        Eigen::Vector4f{uv_array[0].x(),uv_array[0].y(), 0.0f, 0.0f}});
        //
        //mesh_.emplace_back(Vertex{Eigen::Vector4f{coord.x(),
        //                                       -uv_array[2].y() * lheight, 1.0f, 1.0f},
        //                                       Eigen::Vector4f{uv_array[0].x(),uv_array[0].y(), 0.0f, 0.0f}});
        //
        //mesh_.emplace_back(Vertex{Eigen::Vector4f{coord.x(),
        //                                       -uv_array[2].w() * lheight, 1.0f, 1.0f},
        //                                       Eigen::Vector4f{uv_array[0].z(),uv_array[0].w(), 0.0f, 0.0f}});
        //
        //mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[3].x() * lwidth + coord.x(),
        //                                       -uv_array[3].y() * lheight, 1.0f, 1.0f},
        //                                       Eigen::Vector4f{uv_array[1].x(),uv_array[1].y(), 0.0f, 0.0f}});
        //
        //mesh_.emplace_back(Vertex{Eigen::Vector4f{uv_array[3].z() * lwidth + coord.x(),
        //                                       -uv_array[3].w() * lheight, 1.0f, 1.0f},
        //                                       Eigen::Vector4f{uv_array[1].z(),uv_array[1].w(), 0.0f, 0.0f}});

        coord.x() += uv_array[4].x() * lwidth;
    }

    //mesh_.emplace_back(Vertex{Eigen::Vector4f{-1.0f, 0.5f,1.0f,1.0f}, Eigen::Vector4f{0,0,0,0}});
    //mesh_.emplace_back(Vertex{Eigen::Vector4f{-1.0f,-1.0f,1.0f,1.0f}, Eigen::Vector4f{1,0,0,0}});
    //mesh_.emplace_back(Vertex{Eigen::Vector4f{1.0f,0.5f,1.0f,1.0f}, Eigen::Vector4f{0,1,0,0}});
    //mesh_.emplace_back(Vertex{Eigen::Vector4f{1.0f,-1.0f,1.0f,1.0f}, Eigen::Vector4f{1,1,0,0}});
}


}} // namespace
