/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef IJWOAFF_GTL_D3D_FONT_ATLAS_GLYPH_H_
#define IJWOAFF_GTL_D3D_FONT_ATLAS_GLYPH_H_

#include <array>
#include <string>
#include <unordered_map>

namespace gtl {

namespace d3d {

    namespace tags {
        struct xml_format {
        };
    }

    struct Glyph;

    class font_atlas_glyph {

    public:
        font_atlas_glyph(std::wstring filename, gtl::d3d::tags::xml_format);

        font_atlas_glyph() = delete;
        font_atlas_glyph(font_atlas_glyph&&) = default;
        font_atlas_glyph& operator=(font_atlas_glyph&&) = default;

        friend std::ostream& operator<<(std::ostream& s, font_atlas_glyph& f);

        // private:
        std::array<float, 4> padding{}; // padding around each character
        std::string face_name{};        // name of the font
        std::string texture_file{};     // name of the file that contains the font texture
        float size{};                   // font size
        float page_count{};             // number of pages in the definition
        float spacing{};
        float outline{};
        float line_height{};
        float base_width{};
        std::unordered_map<uint8_t, Glyph> glyph_map;
        std::unordered_multimap<uint8_t, std::pair<uint8_t, float>> kerning_map;
    };

    struct Glyph {
        float u, v;               // uv coordinate in texture
        float width, height;      // w/h of glyph
        float x_offset, y_offset; // offset for positioning
        float x_advance;          // xadvance for next character (altered by kerning)
        friend std::ostream& operator<<(std::ostream& s, Glyph& g);
    };
}
} // namespace
#endif
