#ifndef HBWWABVWAFSF_GTL_D3D_IMGUI_ADAPTER_H_
#define HBWWABVWAFSF_GTL_D3D_IMGUI_ADAPTER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)

    class gtl::d3d::imgui_adapter
-----------------------------------------------------------------------------*/

#include <array>
#include <utility>

#include <exception>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <gtl/camera.h>
#include <gtl/d3d_helper_funcs.h>
#include <gtl/d3d_types.h>
#include <gtl/keyboard_enum.h>

//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//
//#include <imgui.h>

#include <gtl/imgui_adapter.h>

namespace gtl {
namespace d3d {

    class imgui_adapter {
        // texture
        // vertices

        constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

        static constexpr unsigned MAX_VERTS = 700;
        static constexpr unsigned MAX_INDICES = 700;

        //struct vertex { float x,y; float u,v; uint32_t color; };
        using vertex_type = ImDrawVert;
        using index_type = ImDrawIdx;

        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;

        gtl::d3d::resource_descriptor_heap cbheap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable cbuffer_;

        gtl::d3d::resource_descriptor_heap vert_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable vert_buffers_;

        d3d::resource_descriptor_heap idx_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable idx_buffers_;

        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;
        gtl::d3d::srv font_texture_;

        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::root_signature root_sig_;
        gtl::d3d::pipeline_state_object pso_;

        std::array<gtl::d3d::direct_command_allocator, frame_count> calloc_;
        std::array<gtl::d3d::graphics_command_list, frame_count> mutable clist_;

        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;

        gtl::d3d::viewport viewport_; //{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
        gtl::d3d::raw::ScissorRect scissor_; //{0,0,960,540};

        gtl::imgui_adapter& imgui_adapter_;
        gtl::imgui_adapter::imgui_data mutable local_imgui_data_;

        std::array<bool, frame_count> mutable imgui_dirty_flags_;
        //unsigned mutable idx_count{},vtx_count{};

        //std::unordered_map<std::string,std::function<void()>> mutable callbacks_;

        //std::string mutable text_box_;

        auto vertex_layout()
        {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                //{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                //{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (size_t)(&((ImDrawVert*)0)->pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (size_t)(&((ImDrawVert*)0)->uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
                //       {"VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                //       {"VERTEX_NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                //       {"VERTEX_BONE_IDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                //       {"VERTEX_BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                //
                //       {"INSTANCE_INFO", 0, DXGI_FORMAT_R16G16B16A16_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                //       {"BONE_ARRAY_OFFSET", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                //       {"BONE_COUNT", 0, DXGI_FORMAT_R32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                //       {"OBJECT_ID", 0, DXGI_FORMAT_R32G32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}
                //  //   {"OBJECT_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                //  //   {"MESH_ID", 0, DXGI_FORMAT_R32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                //  //   {"COLOR_ANGLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
            };
        }

        auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig,
            gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps)
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
            desc_.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
            desc_.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };
            desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            desc_.RasterizerState.FrontCounterClockwise = true;
            desc_.RasterizerState.DepthClipEnable = false;
            desc_.RasterizerState.AntialiasedLineEnable = false;

            desc_.InputLayout.pInputElementDescs = &layout_[0];
            desc_.InputLayout.NumElements = static_cast<unsigned>(layout_.size());

            D3D12_BLEND_DESC blend_desc_ = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            blend_desc_.RenderTarget[0].BlendEnable = true;
            blend_desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blend_desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blend_desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blend_desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blend_desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blend_desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            blend_desc_.RenderTarget[0].BlendEnable = true;

            desc_.BlendState = blend_desc_;

            desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc_.DepthStencilState.DepthEnable = FALSE;
            desc_.DepthStencilState.StencilEnable = FALSE;

            desc_.SampleMask = UINT_MAX;
            desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            desc_.NumRenderTargets = 2;
            desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc_.RTVFormats[1] = DXGI_FORMAT_R32_UINT;
            desc_.SampleDesc.Count = 1;
            return desc_;
        }

        auto sampler_desc()
        {
            D3D12_SAMPLER_DESC sampler_{};
            sampler_.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.MaxAnisotropy = 1;
            sampler_.BorderColor[3] = 0.0f; // no alpha
            sampler_.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            return sampler_;
        }

        //template <typename P>
        //auto get_font_bitmap(P const& point) {
        //    using std::get;
        //
        //    ImGuiIO& io = ImGui::GetIO();
        //    io.DisplaySize = ImVec2(static_cast<float>(get<0>(point)),static_cast<float>(get<1>(point))); // HACK get value from elsewhere..
        //    io.RenderDrawListsFn = NULL;
        //    io.Fonts->TexID = 0;
        //
        //    unsigned char* pixels{};
        //    int width{},height{},bytes_per_pixel{};
        //    //io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height,&bytes_per_pixel);
        //    io.Fonts->GetTexDataAsAlpha8(&pixels,&width,&height,&bytes_per_pixel);
        //    std::vector<uint32_t> font_data;
        //    //width = 500;
        //    //height = 500;
        //
        //    font_data.reserve(width * height);
        //    for (int m = 0; m < (height); ++m) {
        //        for (int n = 0; n < (width); ++n) {
        //            font_data.emplace_back(pixels[m * width + n]);
        //        }
        //    }
        //    auto p = font_data.data();
        //    std::cout << "loaded..\n";
        //    return std::make_tuple(std::move(font_data),static_cast<unsigned>(width),static_cast<unsigned>(height));
        //}

    public:
        imgui_adapter(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue, gtl::imgui_adapter& imgui_)
            : cbheap_{ dev, 3, gtl::d3d::tags::shader_visible{} }
            , cbuffer_{ { { dev, cbheap_.get_handle(0), sizeof(gtl::camera) }, { dev, cbheap_.get_handle(1), sizeof(gtl::camera) }, { dev, cbheap_.get_handle(2), sizeof(gtl::camera) } } }
            ,

            layout_{ vertex_layout() }
            , vert_descriptor_heap_{ dev, 3, gtl::d3d::tags::shader_visible{} }
            , vert_buffers_{ { { dev, vert_descriptor_heap_.get_handle(0), MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{} },
                  { dev, vert_descriptor_heap_.get_handle(1), MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{} },
                  { dev, vert_descriptor_heap_.get_handle(2), MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{} } } }
            , idx_descriptor_heap_{ dev, 3, gtl::d3d::tags::shader_visible{} }
            , idx_buffers_{ { { dev, idx_descriptor_heap_.get_handle(0), MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{} },
                  { dev, idx_descriptor_heap_.get_handle(1), MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{} },
                  { dev, idx_descriptor_heap_.get_handle(2), MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{} } } }
            , texture_descriptor_heap_{ dev, 1, gtl::d3d::tags::shader_visible{} }
            ,
            //font_texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue,
            //    L"D:\\images\\brutal.dds"
            //    },
            font_texture_{ dev, { texture_descriptor_heap_.get_handle(0) }, cqueue, gtl::imgui_adapter::get_font_bitmap(swchain_.dimensions()) }
            , vshader_{ L"imgui_vs.cso" }
            , pshader_{ L"imgui_ps.cso" }
            , root_sig_{ dev, vshader_ }
            , pso_{ dev, pso_desc(dev, root_sig_, vshader_, pshader_) }
            , calloc_{ { { dev }, { dev }, { dev } } }
            , clist_{ { { dev, calloc_[0], pso_ }, { dev, calloc_[1], pso_ }, { dev, calloc_[2], pso_ } } }
            , sampler_heap_{ dev, 1 }
            , sampler_{ dev, sampler_desc(), sampler_heap_->GetCPUDescriptorHandleForHeapStart() }
            , viewport_{ swchain_.viewport() }
            , scissor_{ 0, 0, 960, 540 } 
            , imgui_adapter_{ imgui_ }
        {
            imgui_dirty_flags_.fill(true);
            update(0);
            update(1);
            update(2);
        }

        //void insert_callback(std::string key, std::function<void()> func) {
        //    if (callbacks_.count(key) == 0) {
        //        callbacks_.insert(std::make_pair(std::move(key),std::move(func)));
        //    } else {
        //        throw std::runtime_error{"callback already registered for key"};
        //    }
        //}

        void resize(int w, int h, gtl::d3d::command_queue& cqueue_)
        { // needs dev cqueue etc
            font_texture_ = gtl::d3d::srv{ get_device_from(cqueue_), { texture_descriptor_heap_.get_handle(0) }, cqueue_, gtl::imgui_adapter::get_font_bitmap(std::make_pair(w, h)) };
            viewport_.Width = static_cast<float>(w);
            viewport_.Height = static_cast<float>(h);
            scissor_ = gtl::d3d::raw::ScissorRect{ 0, 0, w, h };
        }
      
        void update(unsigned idx) const
        {
            if (imgui_adapter_.swap_out(local_imgui_data_)) {
                imgui_dirty_flags_.fill(true);
            }

            if (imgui_dirty_flags_[idx]) {

                auto& verts = local_imgui_data_.vertices_;
                auto& indices = local_imgui_data_.indices_;

                vert_buffers_[idx].update(reinterpret_cast<char*>(verts.data()), verts.size() * sizeof(ImDrawVert));
                idx_buffers_[idx].update(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(ImDrawIdx));

                imgui_dirty_flags_[idx] = false;
            }

            //            for (int n = 0, voffs = 0, ioffs = 0; n < draw_data->CmdListsCount; n++) {
            //                auto* cmd_list = draw_data->CmdLists[n];
            //                vert_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->VtxBuffer[0]),
            //                                          cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), voffs * sizeof(ImDrawVert));
            //                idx_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->IdxBuffer[0]),
            //                                         cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), ioffs * sizeof(ImDrawIdx));
            //                voffs += cmd_list->VtxBuffer.size();
            //                ioffs += cmd_list->IdxBuffer.size();
            //            }
            //
            //            idx_count = draw_data->TotalIdxCount;
            //            vtx_count = draw_data->TotalVtxCount;
        }

        void draw(std::vector<ID3D12CommandList*>& v, unsigned idx, float,
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle) const
        {
            update(idx);

            calloc_[idx]->Reset();
            clist_[idx]->Reset(calloc_[idx].get(), nullptr);
            clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

            //imgui_(idx, f, imgui_clist_[idx],viewport_,scissor_,camera,handles,std::addressof(dbview_));

            clist_[idx]->SetPipelineState(pso_.get());
            auto heaps = { sampler_heap_.get(), texture_descriptor_heap_.get() };
            clist_[idx]->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
            //clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            clist_[idx]->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
            clist_[idx]->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            clist_[idx]->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());
            clist_[idx]->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport_), 0);
            //
            //clist_[idx]->SetGraphicsRootShaderResourceView(4,cbuffer_[idx].resource()->GetGPUVirtualAddress());
            //clist_[idx]->SetGraphicsRootShaderResourceView(4,(vert_buffers_[idx].resource())->GetGPUVirtualAddress());
            //clist_[idx]->SetGraphicsRootShaderResourceView(6,(idx_buffers_[idx].resource())->GetGPUVirtualAddress());

            auto viewports = { std::addressof(viewport_) };
            clist_[idx]->RSSetViewports(static_cast<unsigned>(viewports.size()), *viewports.begin());

            //float blendvalues[]{f,f,f,f};
            //clist_[idx]->OMSetBlendFactor(blendvalues);

            clist_[idx]->RSSetScissorRects(1, &scissor_);
            clist_[idx]->OMSetRenderTargets(1, std::addressof(rtv_handle), false, nullptr);

            auto vtx_count = static_cast<unsigned>(local_imgui_data_.vertex_count());
            auto idx_count = static_cast<unsigned>(local_imgui_data_.index_count());

            D3D12_VERTEX_BUFFER_VIEW vview{ vert_buffers_[idx].resource()->GetGPUVirtualAddress(),
                vtx_count * sizeof(vertex_type),
                sizeof(vertex_type) };

            D3D12_INDEX_BUFFER_VIEW ibv{ idx_buffers_[idx].resource()->GetGPUVirtualAddress(),
                idx_count * sizeof(index_type),
                DXGI_FORMAT_R16_UINT }; //DXGI_FORMAT_R32_UINT};

            clist_[idx]->IASetVertexBuffers(0, 1, &vview);
            clist_[idx]->IASetIndexBuffer(&ibv);

            //clist_[idx]->DrawInstanced(6,1,0,0);
            clist_[idx]->DrawIndexedInstanced(idx_count, 1, 0, 0, 0);

            clist_[idx]->Close();

            v.emplace_back(clist_[idx].get());
        }
    };
}
} // namespaces
#endif
