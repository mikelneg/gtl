/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/font_atlas_glyph.h"

#define RAPIDXML_STATIC_POOL_SIZE (256)
#include <RapidXML/rapidxml.hpp>

#include <gtl/file_utilities.h>
#include <gtl/tags.h>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace gtl {
namespace d3d {

    namespace {
        template <typename T>
        T extract_from_stream(std::istream& stream_)
        {
            T t;
            stream_ >> t;
            if (!stream_)
                throw std::runtime_error{"Error parsing stream"};
            return t;
        }

        template <typename T>
        T extract_from_string(std::string string_)
        {
            T t;
            std::stringstream stream_{string_};
            stream_ >> t;
            if (!stream_)
                throw std::runtime_error{"Error parsing stream"};
            return t;
        }

        template <typename T, int N>
        std::array<T, N> extract_from_string(std::string string_, char)
        {
            std::array<T, N> array_;
            std::stringstream stream_{string_};

            for (int i = 0; i < N; ++i)
            {
                std::getline(stream_, string_, ',');
                array_[i] = extract_from_string<T>(string_);

                if (!stream_)
                    throw std::runtime_error{"Error parsing stream"};
            }

            return array_;
        }
    }

    font_atlas_glyph::font_atlas_glyph(std::wstring filename, d3d::tags::xml_format)
    {
        std::vector<char> file = gtl::file::get_file_blob(filename);
        file.push_back(0);
        rapidxml::xml_document<char> doc;
        doc.parse<0>(file.data()); // throws rapidxml::parse_error

        rapidxml::xml_node<>* node = doc.first_node("font");

        node = node->first_node("info");

        face_name = node->first_attribute("face")->value();
        size = extract_from_string<float>(node->first_attribute("size")->value());
        padding = extract_from_string<float, 4>(node->first_attribute("padding")->value(), ',');
        spacing = extract_from_string<float>(node->first_attribute("spacing")->value());
        outline = extract_from_string<float>(node->first_attribute("outline")->value());

        node = node->next_sibling("common");
        //<common lineHeight = "80" base = "57" scaleW = "415" scaleH = "512" pages = "1" packed = "0"/>
        line_height = extract_from_string<float>(node->first_attribute("lineHeight")->value());
        base_width = extract_from_string<float>(node->first_attribute("base")->value());

        page_count = extract_from_string<float>(node->first_attribute("pages")->value());

        if (page_count > 1)
        {
            throw std::runtime_error{"page_count > 1 not currently supported.."};
        }

        node = node->next_sibling("pages");
        texture_file = node->first_node("page")->first_attribute("file")->value();

        // process glyphs
        node = node->next_sibling("chars");
        rapidxml::xml_node<>* glyph_node = node->first_node("char");

        do
        {
            glyph_map.emplace(extract_from_string<unsigned>(glyph_node->first_attribute("id")->value()),
                              Glyph{
                                  extract_from_string<float>(glyph_node->first_attribute("x")->value()), // u
                                  extract_from_string<float>(glyph_node->first_attribute("y")->value()), // v
                                  extract_from_string<float>(glyph_node->first_attribute("width")->value()),
                                  extract_from_string<float>(glyph_node->first_attribute("height")->value()),
                                  extract_from_string<float>(glyph_node->first_attribute("xoffset")->value()),
                                  extract_from_string<float>(glyph_node->first_attribute("yoffset")->value()),
                                  extract_from_string<float>(glyph_node->first_attribute("xadvance")->value())});
        } while (glyph_node = glyph_node->next_sibling());

        node = node->next_sibling("kernings");
        // kernings are only available in non-monospace fonts
        if (node)
        {
            rapidxml::xml_node<>* kerning_node = node->first_node("kerning");
            do
            {
                kerning_map.emplace(extract_from_string<unsigned>(kerning_node->first_attribute("first")->value()),
                                    std::pair<uint8_t, float>{
                                        extract_from_string<unsigned>(kerning_node->first_attribute("second")->value()),
                                        extract_from_string<float>(kerning_node->first_attribute("amount")->value())});
            } while (kerning_node = kerning_node->next_sibling());
        }
    }

    std::ostream& operator<<(std::ostream& s, Glyph& g)
    {
        s << "u,v = " << g.u << "," << g.v << " | width,height = " << g.width << "," << g.height;
        s << " | xoffset, yoffset = " << g.x_offset << "," << g.y_offset << " | xadvance = " << g.x_advance << "\n";
        return s;
    }

    std::ostream& operator<<(std::ostream& s, font_atlas_glyph& f)
    {
        std::cout << "padding = " << f.padding[0] << "," << f.padding[1] << "," << f.padding[2] << "," << f.padding[3] << "\n";
        std::cout << "face_name = " << f.face_name << "\n";
        std::cout << "texture_file = " << f.texture_file << "\n";
        std::cout << "size = " << f.size << "\n";

        std::cout << "page_count = " << f.page_count << "\n";
        std::cout << "spacing = " << f.spacing << "\n";
        std::cout << "outline = " << f.outline << "\n";

        for (auto&& e : f.glyph_map)
        {
            std::cout << static_cast<unsigned>(e.first) << " :: " << e.second;
        }

        for (auto&& e : f.kerning_map)
        {
            std::cout << static_cast<unsigned>(e.first) << "," << static_cast<unsigned>(e.second.first) << " = " << e.second.second << "\n";
        }

        return s;
    }
}
} // namespaces
