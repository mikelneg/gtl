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

    arr[0] = Eigen::Vector4f{g.u, g.v,
                           g.u, g.v + g.height};
    arr[1] = Eigen::Vector4f{g.u + g.width, g.v,
                           g.u + g.width, g.v + g.height};
    

    //<char id = "73" x = "510" y = "218" width = "12" height = "88" xoffset = "11.81" yoffset = "6.69" xadvance = "35.56" page = "0" chnl = "15"/>

    
    arr[2] = Eigen::Vector4f{static_cast<float>(g.x_offset), static_cast<float>(g.y_offset), 
                           static_cast<float>(g.x_offset), static_cast<float>(g.y_offset + g.height)};
    arr[3] = Eigen::Vector4f{static_cast<float>(g.x_offset + g.width), static_cast<float>(g.y_offset), 
                           static_cast<float>(g.x_offset + g.width), static_cast<float>(g.y_offset + g.height)};
                          
    arr[4] = Eigen::Vector4f{g.x_advance,0.0f,0.0f,0.0f};

    return arr;        
}

static 
std::array<Eigen::Vector4f,9> 
character_info(font_atlas_glyph& f, uint8_t c)
{


    using v4 = Eigen::Vector4f; 

    Glyph& g = f.glyph_map.at(c);

    //    1   3   5   // triangle expansion..
    //      \   \
    //    0   2   4

    std::array<Eigen::Vector4f,9> arr 
        {{
                //{0.0f, -g.height,         1.0f,1.0f},    // vertex coordinates in pt space
                //{0.0f,0.0f,              1.0f,1.0f},                                
                //{g.width, -g.height,      1.0f,1.0f},
                //{g.width, 0.0f,          1.0f,1.0f},
                {g.x_offset, g.y_offset + g.height,             1.0f,1.0f},    // vertex coordinates in pt space
                {g.x_offset, g.y_offset,                        1.0f,1.0f},                                
                {g.x_offset + g.width, g.y_offset + g.height,   1.0f,1.0f},
                {g.x_offset + g.width, g.y_offset,              1.0f,1.0f},
        
                {g.u, g.v + g.height,          0.0f, 0.0f},   // uv coordinates in texture space
                {g.u, g.v,                     0.0f, 0.0f},   
                {g.u + g.width, g.v + g.height, 0.0f, 0.0f},
                {g.u + g.width, g.v,            0.0f, 0.0f},                
                {g.x_advance, 0.0f, 0.0f, 0.0f}
        }};      
    return arr;        
}



void font_atlas::construct_vertices(std::string const& message) const
{    
    if (message.empty()) return; 

    // sample:

 //   <font>
 // <info face="font" size="24" bold="1" italic="0" charset="" unicode="" stretchH="100" smooth="1" aa="1" padding="5,5,5,5" spacing="0,0" outline="0"/>
 // <common lineHeight="27" base="18" scaleW="256" scaleH="512" pages="1" packed="0"/>
 // <pages>
 //   <page id="0" file="font.png"/>
 // </pages>
 // <chars count="93">
 //   <char id="97" x="5" y="5" width="17" height="18" xoffset="2" yoffset="5" xadvance="13" page="0" chnl="15"/>
 //   <char id="98" x="5" y="28" width="17" height="22" xoffset="3" yoffset="0" xadvance="15" page="0" chnl="15"/>
 //   <char id="99" x="5" y="55" width="16" height="18" xoffset="2" yoffset="5" xadvance="13" page="0" chnl="15"/>
 //   <char id="100" x="5" y="78" width="17" height="22" xoffset="2" yoffset="0" xadvance="15" page="0" chnl="15"/>
 //   <char id="101" x="26" y="55" width="16" height="18" xoffset="2" yoffset="5" xadvance="13" page="0" chnl="15"/>
 //   <char id="102" x="27" y="5" width="12" height="22" xoffset="1" yoffset="0" xadvance="8" page="0" chnl="15"/>
 //   <char id="103" x="44" y="5" width="17" height="22" xoffset="2" yoffset="5" xadvance="15" page="0" chnl="15"/>
 //   <char id="104" x="5" y="105" width="16" height="22" xoffset="3" yoffset="0" xadvance="15" page="0" chnl="15"/>
 //   <char id="105" x="5" y="132" width="8" height="22" xoffset="3" yoffset="0" xadvance="7" page="0" chnl="15"/>
 //   <char id="106" x="5" y="159" width="10" height="27" xoffset="1" yoffset="0" xadvance="7" page="0" chnl="15"/>
 //   <char id="107" x="18" y="132" width="16" height="22" xoffset="3" yoffset="0" xadvance="13" page="0" chnl="15"/>
 //   <char id="108" x="26" y="105" width="8" height="22" xoffset="3" yoffset="0" xadvance="7" page="0" chnl="15"/>
 //   <char id="109" x="27" y="32" width="23" height="17" xoffset="3" yoffset="5" xadvance="21" page="0" chnl="15"/>

    // vertices are in pt space..

    //
    //    1   3   5   // triangle expansion..
    //      \   \
    //    0   2   4
       
    //std::vector<Vertex> mesh;

    mesh_.clear();

    using v2 = Eigen::Vector2f;
    using v4 = Eigen::Vector4f;

    v4 coord{0.0f, 0.0f, 0.0f, 0.0f};
    float lwidth = font_definition.base_width;
    float lheight = font_definition.line_height;

    auto insert_glyph = [this](auto&& uv, auto& coord){                
                
        mesh_.emplace_back(Vertex{uv[0]+coord, uv[4]});        // degenerate strip spans last-to-first 
        
        mesh_.emplace_back(Vertex{uv[0]+coord, uv[4]});               
        mesh_.emplace_back(Vertex{uv[1]+coord, uv[5]});       
        mesh_.emplace_back(Vertex{uv[2]+coord, uv[6]});       
        mesh_.emplace_back(Vertex{uv[3]+coord, uv[7]});       

        mesh_.emplace_back(Vertex{uv[3]+coord, uv[7]});   

        coord.x() += uv[8].x();
    };                     

    insert_glyph( character_info(font_definition, message[0]), coord );
    //coord.x() += lwidth;
    
    for (unsigned i = 1; i < message.size(); ++i) {        
        
        auto it = font_definition.kerning_map.equal_range(message[i-1]);
        for (auto x = it.first; x != it.second; ++x) {
            if (x->second.first == message[i]) {
                coord.x() += x->second.second;
                break;
            }
        }

        insert_glyph( character_info(font_definition, message[i]), coord );                
    }
    
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


        coord.x() += uv_array[4].x() * lwidth;
    }
}


}} // namespace
