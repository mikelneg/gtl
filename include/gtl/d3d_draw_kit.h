/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef WOIAJFAJBOAWFAW_GTL_D3D_DRAW_KIT_H_
#define WOIAJFAJBOAWFAW_GTL_D3D_DRAW_KIT_H_

#include <array>
#include <utility>
#include <cstdint>

#include <exception>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <gtl/camera.h>
#include <gtl/d3d_helper_funcs.h>
#include <gtl/d3d_types.h>
#include <gtl/win_keyboard.h>

#include <gtl/draw_kit.h>

#include <Eigen/Geometry>

namespace gtl {
namespace d3d {

    class draw_kit {
        using vertex_type = gtl::draw_data::vertex_type;
        using index_type = gtl::draw_data::index_type;

        constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

        static constexpr unsigned MAX_VERTS = 20000;
        static constexpr unsigned MAX_INDICES = 50000;
        
        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;

        gtl::d3d::resource_descriptor_heap cbheap_;
        gtl::d3d::constant_buffer mutable cbuffer_;

        gtl::d3d::resource_descriptor_heap vert_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable vert_buffers_;

        d3d::resource_descriptor_heap idx_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable idx_buffers_;

        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;        

        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::root_signature root_sig_;
        gtl::d3d::pipeline_state_object pso_;

        std::array<gtl::d3d::direct_command_allocator, frame_count> calloc_;
        std::array<gtl::d3d::graphics_command_list, frame_count> mutable clist_;

        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;

        gtl::d3d::viewport viewport_;        //{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
        gtl::d3d::raw::ScissorRect scissor_; //{0,0,960,540};

        gtl::draw_kit& draw_kit_;
        gtl::draw_data mutable local_draw_data_;

        std::array<bool, frame_count> mutable draw_data_dirty_flags_;
        // unsigned mutable idx_count{},vtx_count{};

        // std::unordered_map<std::string,std::function<void()>> mutable callbacks_;

        // std::string mutable text_box_;

        auto vertex_layout()
        {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},                
                {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}                
            };
        }

        auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig, gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps)
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
            desc_.VS = {reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize()};
            desc_.PS = {reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize()};
            desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            desc_.RasterizerState.FrontCounterClockwise = false;
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

            blend_desc_.RenderTarget[1].BlendEnable = false;

            desc_.BlendState = blend_desc_;

            desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc_.DepthStencilState.DepthEnable = FALSE;
            desc_.DepthStencilState.StencilEnable = FALSE;

            desc_.SampleMask = UINT_MAX;
            desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            desc_.NumRenderTargets = 1;
            desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            // desc_.RTVFormats[1] = DXGI_FORMAT_R32_UINT;
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

    public:
        draw_kit(gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue, gtl::draw_kit& draw_data_)
            : draw_kit(get_device_from(swchain_), swchain_, cqueue, draw_data_)
        {}

        draw_kit(gtl::d3d::device& dev, gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue, gtl::draw_kit& draw_data_)
            : cbheap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              cbuffer_{dev, cbheap_.get_handle(0), sizeof(Eigen::Matrix4f) * 2},
              layout_{vertex_layout()},
              vert_descriptor_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              vert_buffers_{{{dev, vert_descriptor_heap_.get_handle(0), MAX_VERTS * sizeof(vertex_type), gtl::d3d::tags::shader_view{}},
                             {dev, vert_descriptor_heap_.get_handle(1), MAX_VERTS * sizeof(vertex_type), gtl::d3d::tags::shader_view{}},
                             {dev, vert_descriptor_heap_.get_handle(2), MAX_VERTS * sizeof(vertex_type), gtl::d3d::tags::shader_view{}}}},
              idx_descriptor_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              idx_buffers_{{{dev, idx_descriptor_heap_.get_handle(0), MAX_INDICES * sizeof(index_type), gtl::d3d::tags::shader_view{}},
                            {dev, idx_descriptor_heap_.get_handle(1), MAX_INDICES * sizeof(index_type), gtl::d3d::tags::shader_view{}},
                            {dev, idx_descriptor_heap_.get_handle(2), MAX_INDICES * sizeof(index_type), gtl::d3d::tags::shader_view{}}}},
              texture_descriptor_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},              
              vshader_{L"box2d_vs.cso"},    // HACK change name
              pshader_{L"box2d_ps.cso"},
              root_sig_{dev, vshader_},
              pso_{dev, pso_desc(dev, root_sig_, vshader_, pshader_)},
              calloc_{{{dev}, {dev}, {dev}}},
              clist_{{{dev, calloc_[0], pso_}, {dev, calloc_[1], pso_}, {dev, calloc_[2], pso_}}},
              sampler_heap_{dev, 1},
              sampler_{dev, sampler_desc(), sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
              viewport_{swchain_.viewport()},
              scissor_{0, 0, 960, 540},
              draw_kit_{draw_data_}
        {
            draw_data_dirty_flags_.fill(true);
            update(0);
            update(1);
            update(2);

            initialize_null_descriptor_srv(dev, texture_descriptor_heap_.get_handle(1));
            initialize_null_descriptor_uav(dev, texture_descriptor_heap_.get_handle(2));
        }      

        void resize(int w, int h, gtl::d3d::command_queue& cqueue_)
        { // needs dev cqueue etc            
            viewport_.Width = static_cast<float>(w);
            viewport_.Height = static_cast<float>(h);
            scissor_ = gtl::d3d::raw::ScissorRect{0, 0, w, h};
        }

        void update(unsigned idx) const
        {
            if (draw_kit_.swap_out(local_draw_data_))
            {
                draw_data_dirty_flags_.fill(true);
            }

            if (draw_data_dirty_flags_[idx])
            {

                auto& verts = local_draw_data_.vertices();
                auto& indices = local_draw_data_.indices();

                vert_buffers_[idx].update(reinterpret_cast<char*>(verts.data()), local_draw_data_.vertex_count() * sizeof(vertex_type));
                idx_buffers_[idx].update(reinterpret_cast<char*>(indices.data()), local_draw_data_.index_count() * sizeof(index_type));

                draw_data_dirty_flags_[idx] = false;
            }

            //            for (int n = 0, voffs = 0, ioffs = 0; n < draw_data->CmdListsCount; n++) {
            //                auto* cmd_list = draw_data->CmdLists[n];
            //                vert_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->VtxBuffer[0]),
            //                                          cmd_list->VtxBuffer.size() * sizeof(vertex_type), voffs *
            //                                          sizeof(vertex_type));
            //                idx_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->IdxBuffer[0]),
            //                                         cmd_list->IdxBuffer.size() * sizeof(index_type), ioffs *
            //                                         sizeof(index_type));
            //                voffs += cmd_list->VtxBuffer.size();
            //                ioffs += cmd_list->IdxBuffer.size();
            //            }
            //
            //            idx_count = draw_data->TotalIdxCount;
            //            vtx_count = draw_data->TotalVtxCount;
        }

        void draw(std::vector<ID3D12CommandList*>& v, unsigned idx, float, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle, 
                  Eigen::Matrix4f const& camera, Eigen::Matrix4f const& proj) const
        {
            update(idx);

            //
            cbuffer_.update(reinterpret_cast<const char*>(&camera), sizeof(camera));                   
            cbuffer_.update(reinterpret_cast<const char*>(&proj), sizeof(proj), sizeof(camera));                   


            calloc_[idx]->Reset();
            clist_[idx]->Reset(calloc_[idx].get(), nullptr);
            clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

            // draw_data_(idx, f, draw_data_clist_[idx],viewport_,scissor_,camera,handles,std::addressof(dbview_));

            clist_[idx]->SetPipelineState(pso_.get());
            auto heaps = {sampler_heap_.get(), texture_descriptor_heap_.get()};
            clist_[idx]->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
            // clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);                   

            clist_[idx]->SetGraphicsRootConstantBufferView(0, (cbuffer_.resource())->GetGPUVirtualAddress());
            clist_[idx]->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            clist_[idx]->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());
            clist_[idx]->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport_), 0);
            //
            // clist_[idx]->SetGraphicsRootShaderResourceView(4,cbuffer_[idx].resource()->GetGPUVirtualAddress());
            // clist_[idx]->SetGraphicsRootShaderResourceView(4,(vert_buffers_[idx].resource())->GetGPUVirtualAddress());
            // clist_[idx]->SetGraphicsRootShaderResourceView(6,(idx_buffers_[idx].resource())->GetGPUVirtualAddress());

            auto viewports = {std::addressof(viewport_)};
            clist_[idx]->RSSetViewports(static_cast<unsigned>(viewports.size()), *viewports.begin());

            // float blendvalues[]{f,f,f,f};
            // clist_[idx]->OMSetBlendFactor(blendvalues);

            clist_[idx]->RSSetScissorRects(1, &scissor_);
            clist_[idx]->OMSetRenderTargets(1, std::addressof(rtv_handle), false, nullptr);

            auto vtx_count = static_cast<unsigned>(local_draw_data_.vertex_count());
            auto idx_count = static_cast<unsigned>(local_draw_data_.index_count());

            D3D12_VERTEX_BUFFER_VIEW vview{vert_buffers_[idx].resource()->GetGPUVirtualAddress(), vtx_count * sizeof(vertex_type), sizeof(vertex_type)};

            D3D12_INDEX_BUFFER_VIEW ibv{idx_buffers_[idx].resource()->GetGPUVirtualAddress(), idx_count * sizeof(index_type), DXGI_FORMAT_R16_UINT}; // DXGI_FORMAT_R32_UINT};

            clist_[idx]->IASetVertexBuffers(0, 1, &vview);
            clist_[idx]->IASetIndexBuffer(&ibv);

            // clist_[idx]->DrawInstanced(6,1,0,0);
            clist_[idx]->DrawIndexedInstanced(idx_count, 1, 0, 0, 0);

            clist_[idx]->Close();

            v.emplace_back(clist_[idx].get());
        }
    };
}
} // namespaces
#endif
