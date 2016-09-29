/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BBWUWOAIFABBSDFSW_GTL_IMGUI_ADAPTER_H_
#define BBWUWOAIFABBSDFSW_GTL_IMGUI_ADAPTER_H_

#include <cassert>
#include <utility>
#include <vector>
#include <chrono>

#include <algorithm>
#include <string>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <imgui.h>

#include <gtl/events.h>
#include <gtl/win_keyboard.h>
#include <vn/swap_object.h>
#include <vn/boost_variant_utilities.h>

namespace gtl {

class imgui_adapter {

public:
    struct imgui_data {
        std::vector<ImDrawVert> vertices_;
        std::vector<ImDrawIdx> indices_;

        std::size_t vertex_count_;
        std::size_t index_count_;

        auto vertex_count() const noexcept
        {
            return (std::min)(vertex_count_, vertices_.size());
        }
        auto index_count() const noexcept
        {
            return (std::min)(index_count_, indices_.size());
        }
        void swap(imgui_data& other)
        {
            using std::swap;
            swap(vertices_, other.vertices_);
            swap(indices_, other.indices_);
        }
    };

private:
    // TODO implement copy-free buffer updating (instead of swap_object<vector> or whatever)
    vn::swap_object<imgui_data> output_buffer_; 

    imgui_data local_data_;

    std::vector<char> text_box_a_;
    std::vector<char> text_box_b_;
    
    ImVec2 dpadoffset{};

    bool mutable scene_dirty_ = true;

public:
    imgui_adapter()
    {
        std::string s{"hi there.."};
        text_box_a_.insert(end(text_box_a_), begin(s), end(s));
        text_box_b_.insert(end(text_box_b_), begin(s), end(s));
        text_box_a_.resize(256);
        text_box_b_.resize(256);

        auto& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(960.0f, 540.0f); // HACK get value from elsewhere..
        io.RenderDrawListsFn = NULL;
        io.Fonts->TexID = 0;

        io.KeyMap[ImGuiKey_Backspace] = value(keyboard::Backspace);
        io.KeyMap[ImGuiKey_Enter] = value(keyboard::Enter);
    }

    void dump_data(ImDrawData& draw_data)
    {
        using std::begin;
        using std::end;

        auto& verts = local_data_.vertices_;
        auto& indices = local_data_.indices_;

        if (static_cast<int>(verts.size()) < draw_data.TotalVtxCount)
            verts.resize(draw_data.TotalVtxCount);
        if (static_cast<int>(indices.size()) < draw_data.TotalIdxCount)
            indices.resize(draw_data.TotalIdxCount);

        local_data_.vertex_count_ = 0;
        local_data_.index_count_ = 0;

        for (int n = 0, voffs = 0, ioffs = 0; n < draw_data.CmdListsCount; n++)
        {
            auto* cmd_list = draw_data.CmdLists[n];

            assert(voffs < static_cast<int>(verts.size()));
            assert(ioffs < static_cast<int>(indices.size()));

            std::copy(begin(cmd_list->VtxBuffer), end(cmd_list->VtxBuffer), begin(verts) + voffs);            
            std::transform(begin(cmd_list->IdxBuffer), end(cmd_list->IdxBuffer), begin(indices) + ioffs, 
                           [&](auto const& v) { return v + voffs; });            

            voffs += cmd_list->VtxBuffer.size();
            ioffs += cmd_list->IdxBuffer.size();

            local_data_.vertex_count_ += cmd_list->VtxBuffer.size();
            local_data_.index_count_ += cmd_list->IdxBuffer.size();            
        }

        output_buffer_.swap_in(local_data_);
    }

    bool swap_out(imgui_data& external)
    {
        return output_buffer_.swap_out(external);
    }

    bool dirty() const { return scene_dirty_; }
    void set_dirty() const { scene_dirty_ = true; }
    void clear_dirty() const { scene_dirty_ = false; }

    void render(float dt)
    {
        auto& io = ImGui::GetIO();

        io.DeltaTime = dt;

    //if (dirty()) {

        ImGui::NewFrame();

        ImGui::Begin("Window Title");

        // ImGui::PushID(55);
        if (ImGui::Button("Button"))
        {
            std::cout << "resize button pressed..\n";
        }
        // ImGui::PopID();
        ImGui::PushID(44);
        ImGui::InputText("input:", text_box_a_.data(), text_box_a_.size());
        ImGui::PopID();
        if (ImGui::IsItemActive())
        {
            // callbacks_["steal_focus"]();
        }
        else
        {
            // callbacks_["return_focus"]();
        }
        ImGui::InputText("other:", text_box_b_.data(), text_box_b_.size());
        ImGui::End();


        ImGui::SetNextWindowContentSize(ImVec2{100.0f,100.0f});
        ImGui::SetNextWindowSizeConstraints(ImVec2{100.0f,100.0f},ImVec2{160.0f,160.0f});
        
        if (ImGui::Begin("Controller Stick")) {        
            ImGui::Text("SomeText..");                

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            ImU32 col32 = ImColor{1.0f,1.0f,1.0f,1.0f};

            auto winpos = ImGui::GetWindowPos();
            auto wincent = ImGui::GetWindowSize();
            
            winpos.x += (wincent.x / 2.0f);
            winpos.y += (wincent.y / 2.0f);

            draw_list->AddCircle(winpos, 21.21f, col32); 

            winpos.x += dpadoffset.x;
            winpos.y += dpadoffset.y;

            col32 = ImColor{1.0f,0.0f,0.4f,1.0f};

            draw_list->AddCircle(winpos, 5.0f, col32, 4, 0.6f);

                //draw_list->AddCircle(ImVec2(x+sz*0.5f, y+sz*0.5f), sz*0.5f, col32, 20, thickness); x += sz+spacing;
                //draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 0.0f, ~0, thickness); x += sz+spacing;
                //draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f, ~0, thickness); x += sz+spacing;
                //draw_list->AddTriangle(ImVec2(x+sz*0.5f, y), ImVec2(x+sz,y+sz-0.5f), ImVec2(x,y+sz-0.5f), col32, thickness); x += sz+spacing;
                //draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y   ), col32, thickness); x += sz+spacing;
                //draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, thickness); x += sz+spacing;
                //draw_list->AddLine(ImVec2(x, y), ImVec2(x,    y+sz), col32, thickness); x += spacing;
                //draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x+sz*1.3f,y+sz*0.3f), ImVec2(x+sz-sz*1.3f,y+sz-sz*0.3f), ImVec2(x+sz, y+sz), col32, thickness);
                //x = p.x + 4;
                //y += sz+spacing;

        }
        
        ImGui::End();

        ImGui::Render();

        dump_data(*ImGui::GetDrawData());

    //    clear_dirty();
    // }
    }

    static std::tuple<std::vector<uint32_t>,
                      unsigned, // width
                      unsigned  // height
                      >
    get_font_bitmap(std::pair<int, int> display_dimensions)
    {
        auto& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(display_dimensions.first);
        io.DisplaySize.y = static_cast<float>(display_dimensions.second);
        //
        io.RenderDrawListsFn = NULL;
        io.Fonts->TexID = 0;

        unsigned char* pixels{};
        int width{}, height{}, bytes_per_pixel{};
        // io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height,&bytes_per_pixel);
        io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height, &bytes_per_pixel);

        std::vector<uint32_t> font_data;

        for (int m = 0; m < height; ++m)
        {
            for (int n = 0; n < width; ++n)
            {
                font_data.emplace_back(pixels[m * width + n]);
            }
        }

        return std::make_tuple(std::move(font_data), static_cast<unsigned>(width), static_cast<unsigned>(height));
    }

    void dpad_offset(float x, float y) {
        dpadoffset = ImVec2{x,y};
    }
    
    void dispatch_event(gtl::event const& e) const {
        using boost::apply_visitor;
        apply_visitor(*this,e);
    }

    template <typename T>
    void operator()(T const&) const {}

    void operator()(gtl::events::mouse_event const& e) const {
        using namespace gtl::events;
        
        auto handler 
        = vn::make_lambda_visitor(
            [this](mouse_lbutton_up const& m) { set_dirty(); this->mouse_up(static_cast<float>(m.x),static_cast<float>(m.y)); },
            [this](mouse_lbutton_down const& m) { set_dirty(); this->mouse_down(static_cast<float>(m.x),static_cast<float>(m.y)); },
            [this](mouse_moved const& m) { set_dirty(); this->mouse_move(static_cast<float>(m.x),static_cast<float>(m.y)); },
            [](auto const&) {}
            );

        handler(e);
    }


    static void mouse_up(float x, float y)
    {
        auto& io = ImGui::GetIO();
        io.MousePos.x = x;
        io.MousePos.y = y;
        io.MouseDown[0] = false;               
    }

    static void mouse_down(float x, float y)
    {
        auto& io = ImGui::GetIO();
        io.MousePos.x = x;
        io.MousePos.y = y;
        io.MouseDown[0] = true;
    }

    static void mouse_move(float x, float y)
    {
        auto& io = ImGui::GetIO();
        io.MousePos.x = x;
        io.MousePos.y = y;
    }

    static void add_input_charcter(ImWchar c)
    {
        auto& io = ImGui::GetIO();
        io.AddInputCharacter(c);
    }

    static void key_up(gtl::keyboard k)
    {
        auto& io = ImGui::GetIO();
        io.KeysDown[reinterpret_cast<std::underlying_type_t<decltype(k)>&>(k)] = false;
    }

    static void key_down(gtl::keyboard k)
    {
        auto& io = ImGui::GetIO();
        io.KeysDown[reinterpret_cast<std::underlying_type_t<decltype(k)>&>(k)] = true;
    }
};

/* Old reference code...

    t_buffers_{{{dev,vert_descriptor_heap_.get_handle(0),MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{}},
                           {dev,vert_descriptor_heap_.get_handle(1),MAX_VERTS * sizeof(ImDrawVert),
gtl::d3d::tags::shader_view{}},
                           {dev,vert_descriptor_heap_.get_handle(2),MAX_VERTS * sizeof(ImDrawVert),
gtl::d3d::tags::shader_view{}}}},
            idx_descriptor_heap_{dev,3,gtl::d3d::tags::shader_visible{}},
            idx_buffers_{{{dev,idx_descriptor_heap_.get_handle(0),MAX_INDICES * sizeof(ImDrawIdx),
gtl::d3d::tags::shader_view{}},


        void insert_callback(std::string key, std::function<void()> func) {
            if (callbacks_.count(key) == 0) {
                callbacks_.insert(std::make_pair(std::move(key),std::move(func)));
            } else {
                throw std::runtime_error{"callback already registered for key"};
            }
        }

        void resize(int w, int h, gtl::d3d::command_queue& cqueue_) { // needs dev cqueue etc
            font_texture_ = gtl::d3d::srv{get_device_from(cqueue_), {texture_descriptor_heap_.get_handle(0)}, cqueue_,
get_font_bitmap(std::make_pair(w,h))};
            viewport_.Width = static_cast<float>(w);
            viewport_.Height = static_cast<float>(h);
            scissor_ = gtl::d3d::raw::ScissorRect{0,0,w,h};
        }

        void mouse_up(float x, float y) const {
            auto& io = ImGui::GetIO();
            io.MousePos.x = static_cast<float>(x);
            io.MousePos.y = static_cast<float>(y);
            io.MouseDown[0] = false;
        }

        void mouse_down(float x, float y) const {
            auto& io = ImGui::GetIO();
            io.MousePos.x = static_cast<float>(x);
            io.MousePos.y = static_cast<float>(y);
            io.MouseDown[0] = true;
        }

        void mouse_move(float x, float y) const {
            auto& io = ImGui::GetIO();
            io.MousePos.x = static_cast<float>(x);
            io.MousePos.y = static_cast<float>(y);
        }

        void add_input_charcter(unsigned c) const {
            auto& io = ImGui::GetIO();
            io.AddInputCharacter(static_cast<ImWchar>(c));
        }

        void key_up(gtl::keyboard::keyboard_enum k) const {
            auto& io = ImGui::GetIO();
            io.KeysDown[reinterpret_cast<std::underlying_type_t<decltype(k)>&>(k)] = false;
        }

        void key_down(gtl::keyboard::keyboard_enum k) const {
            auto& io = ImGui::GetIO();
            io.KeysDown[reinterpret_cast<std::underlying_type_t<decltype(k)>&>(k)] = true;
        }

        void imgui_render() const {
            //auto& io = ImGui::GetIO();
            //io.MouseDrawCursor = true;

            ImGui::NewFrame();

            ImGui::Begin("Window Title Here");

            //ImGui::PushID(55);
            if (ImGui::Button("resize")) {
                std::cout << "resize button pressed..\n";
            }
            //ImGui::PopID();
            ImGui::PushID(44);
            ImGui::InputText("input:",text_box_.data(),text_box_.size());
            ImGui::PopID();
            if (ImGui::IsItemActive()) {
                callbacks_["steal_focus"]();
            } else {
                callbacks_["return_focus"]();
            }
            ImGui::InputText("other:",other_box_.data(),other_box_.size());

            ImGui::End();

            ImGui::Render();                  // TODO implement copy-free buffer updating (instead of
swap_object<vector> or whatever)
        }

//    ImVec2      MousePos;                   // Mouse position, in pixels (set to -1,-1 if no mouse / on another
screen, etc.)
//    bool        MouseDown[5];               // Mouse buttons: left, right, middle + extras. ImGui itself mostly only
uses left button (BeginPopupContext** are using right button). Others buttons allows us to track if the mouse is being
used by your application + available to user as a convenience via IsMouse** API.
//    float       MouseWheel;                 // Mouse wheel: 1 unit scrolls about 5 lines text.
//    bool        MouseDrawCursor;            // Request ImGui to draw a mouse cursor for you (if you are on a platform
without a mouse cursor).
//    bool        KeyCtrl;                    // Keyboard modifier pressed: Control
//    bool        KeyShift;                   // Keyboard modifier pressed: Shift
//    bool        KeyAlt;                     // Keyboard modifier pressed: Alt
//    bool        KeySuper;                   // Keyboard modifier pressed: Cmd/Super/Windows
//    bool        KeysDown[512];              // Keyboard keys that are pressed (in whatever storage order you naturally
have access to keyboard data)
//    ImWchar     InputCharacters[16+1];      // List of characters input (translated by user from keypress+keyboard
state). Fill using AddInputCharacter() helper.


        void update(unsigned idx, ImDrawData* draw_data) const
        {
            // HACK no bounds checking currently..
            //idx_count = 0;
            //vtx_count = 0;

            for (int n = 0, voffs = 0, ioffs = 0; n < draw_data->CmdListsCount; n++) {
                auto* cmd_list = draw_data->CmdLists[n];
                vert_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->VtxBuffer[0]),
                                          cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), voffs * sizeof(ImDrawVert));
                idx_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->IdxBuffer[0]),
                                         cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), ioffs * sizeof(ImDrawIdx));
                voffs += cmd_list->VtxBuffer.size();
                ioffs += cmd_list->IdxBuffer.size();
            }

            idx_count = draw_data->TotalIdxCount;
            vtx_count = draw_data->TotalVtxCount;
        }

     */

} // namespaces
#endif
