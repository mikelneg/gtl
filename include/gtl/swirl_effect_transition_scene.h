/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_
#define UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_

#include <gtl/events.h>
#include <gtl/keyboard_enum.h>

#include <Windows.h>
#include <windowsx.h>

#include <array>
#include <cstddef>
#include <utility>

#include <gtl/copyable_atomic.h>
#include <gtl/d3d_helper_funcs.h>
#include <gtl/d3d_types.h>
#include <gtl/gtl_window.h>

#include <gtl/mesh_group.h>
#include <gtl/gui_rect_draw.h>
#include <gtl/object_rendering_scene.h>

#include <vn/math_utilities.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <atomic>
#include <cmath>
#include <random>
#include <vector>

#include <gtl/command_variant.h>
#include <gtl/stage.h>

#include <gtl/physics/simulation_interface.h>

namespace gtl {
namespace scenes {
    namespace transitions {

        inline float pi()
        {
            return std::atan2(0.0f, -1.0f);
        }

        inline float to_radians(float degrees)
        {
            return (degrees * (pi() / 180.f));
        }

        inline float to_degrees(float radians)
        {
            return (radians * (180.0f / pi()));
        }

        inline Eigen::Matrix4f makeProjectionMatrix(float fov_y, float aspect_ratio, float z_near, float z_far)
        {
            float s = 1.0f / std::tan(fov_y * 0.5f);
            Eigen::Matrix4f matrix_;

            //  f(z,near,far) = [ z * (1/(far-near)) + (-near/(far-near)) ] / z
            //  maps z between near and far to [0...1]
            //  recommended that you graph this with your near/far values to examine
            //  the scale

            matrix_ << s / aspect_ratio, 0.0f, 0.0f, 0.0f, 0.0f, s, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (z_far - z_near), 1.0f, 0.0f, 0.0f, -z_near / (z_far - z_near), 0.0f;
            return matrix_;
        }

        template <typename T> // HACK clean this up..
        void update(T& cb, float wh_ratio)
        {
            using namespace Eigen;
            static Quaternionf orientation_{Quaternionf::Identity().normalized()};
            static Quaternionf rot_{Quaternionf::FromTwoVectors(Vector3f{0.0f, 0.0f, 1.0f}, Vector3f{0.0001f, 0.0001f, 1.0f}.normalized()).normalized()};

            static auto const proj_mat = makeProjectionMatrix(to_radians(30.0f), wh_ratio, 0.0001f, 1.0f);

            orientation_ = orientation_ * rot_;
            Affine3f transform_{Affine3f::Identity()};
            transform_.rotate(orientation_.toRotationMatrix());

            cb.view_matrix = transform_.matrix() * proj_mat;
        };

        struct cbuffer { // HACK fix this..
            Eigen::Matrix4f view_matrix{};

            cbuffer(float wh_ratio)
            {
                update(*this, wh_ratio);
            }
        };

        class swirl_effect {

            constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

            gtl::d3d::vertex_shader vshader_;
            gtl::d3d::pixel_shader pshader_;
            gtl::d3d::root_signature root_sig_;

            gtl::d3d::resource_descriptor_heap cbheap_;

            cbuffer mutable cbuf_;
            // std::array<gtl::d3d::constant_buffer, frame_count> mutable cbuffer_;
            gtl::d3d::constant_buffer mutable cbuffer_;

            gtl::d3d::depth_stencil_buffer depth_buffers_;

            gtl::d3d::pipeline_state_object pso_;

            std::array<gtl::d3d::direct_command_allocator, frame_count> calloc_;
            std::array<gtl::d3d::graphics_command_list, frame_count> clist_;
            std::array<gtl::d3d::graphics_command_list, frame_count> mutable gui_rect_clist_;
            std::array<gtl::d3d::graphics_command_list, frame_count> mutable object_effect_clist_;
            // std::array<gtl::d3d::graphics_command_list,frame_count> mutable imgui_clist_;
            std::array<gtl::d3d::graphics_command_list, frame_count> mutable id_sampler_clist_;

            // std::array<std::atomic<int32_t>, frame_count> mutable ids_;

            gtl::d3d::viewport viewport_;        //{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
            gtl::d3d::raw::ScissorRect scissor_; //{0,0,960,540};

            gtl::d3d::resource_descriptor_heap resource_heap_;
            gtl::d3d::srv texture_;
            gtl::d3d::rtv_srv_texture2D mutable id_layer_;
            gtl::d3d::resource mutable id_readback_;

            gtl::d3d::sampler_descriptor_heap sampler_heap_;
            gtl::d3d::sampler sampler_;

            gtl::d3d::rect_draw gui_rects_;
            gtl::d3d::object_rendering_scene object_effect_;
            // gtl::d3d::imgui_adapter imgui_;

            gtl::copyable_atomic<int64_t> mutable mouse_coord_;

            auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig, gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps)
            {
                D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
                desc_.pRootSignature = rsig.get();
                desc_.VS = {reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize()};
                desc_.PS = {reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize()};
                desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

                desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
                desc_.RasterizerState.FrontCounterClockwise = false;
                desc_.RasterizerState.DepthClipEnable = true;
                desc_.RasterizerState.AntialiasedLineEnable = true;

                D3D12_BLEND_DESC blend_desc_ = {}; // CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                // blend_desc_.RenderTarget[0].BlendEnable = true; // HACK testing..
                blend_desc_.RenderTarget[0].BlendEnable = false;
                blend_desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

                blend_desc_.RenderTarget[1].BlendEnable = false;

                desc_.BlendState = blend_desc_;

                desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
                // desc_.DepthStencilState.FrontFace.StencilDepthFailOp;
                desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
                desc_.DepthStencilState.DepthEnable = TRUE;
                desc_.DepthStencilState.StencilEnable = FALSE;

                desc_.SampleMask = UINT_MAX;
                desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                // desc_.NumRenderTargets = 1; // swap chain & id system..
                desc_.NumRenderTargets = 2;
                desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc_.RTVFormats[1] = DXGI_FORMAT_R32_UINT;
                desc_.SampleDesc.Count = 1;
                return desc_;
            }

        public:
            template <typename M>
            swirl_effect(gtl::d3d::device& dev_, gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue_, gtl::physics::simulation& physics_, M& mesh_group_)
                : vshader_{L"skybox_vs.cso"},
                  pshader_{L"skybox_ps.cso"},
                  root_sig_{dev_, vshader_},
                  cbheap_{dev_, frame_count, gtl::d3d::tags::shader_visible{}},
                  cbuf_{960.0f / 540.0f},
                  cbuffer_{dev_, cbheap_.get_handle(0), sizeof(cbuf_)},
                  //          {dev_, cbheap_.get_handle(1), sizeof(cbuf_)},
                  //          {dev_, cbheap_.get_handle(2), sizeof(cbuf_)}}},
                  depth_buffers_{swchain_},
                  pso_{dev_, pso_desc(dev_, root_sig_, vshader_, pshader_)},
                  calloc_{{{dev_}, {dev_}, {dev_}}},
                  clist_{{{dev_, calloc_[0], pso_}, {dev_, calloc_[1], pso_}, {dev_, calloc_[2], pso_}}},
                  gui_rect_clist_{{{dev_, calloc_[0]}, {dev_, calloc_[1]}, {dev_, calloc_[2]}}},
                  object_effect_clist_{{{dev_, calloc_[0]}, {dev_, calloc_[1]}, {dev_, calloc_[2]}}},
                  // imgui_clist_{{{dev_,calloc_[0]},{dev_,calloc_[1]},{dev_,calloc_[2]}}},
                  id_sampler_clist_{{{dev_, calloc_[0]}, {dev_, calloc_[1]}, {dev_, calloc_[2]}}},
                  viewport_{swchain_.viewport()}, // {0.0f,0.0f,960.0f,540.0f,0.0f,1.0f},
                  scissor_{0, 0, 960, 540},       // HACK fixed values..
                  resource_heap_{dev_, 3, gtl::d3d::tags::shader_visible{}},
                  texture_{dev_, {resource_heap_->GetCPUDescriptorHandleForHeapStart()}, cqueue_, L"data\\images\\skyboxes\\Nightsky.dds"},
                  id_layer_{swchain_, DXGI_FORMAT_R32_UINT, 3, gtl::d3d::tags::shader_visible{}},
                  sampler_heap_{dev_, 1},
                  sampler_{dev_, sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
                  gui_rects_{dev_, cqueue_, root_sig_, physics_},
                  object_effect_{dev_, cqueue_, root_sig_, physics_,mesh_group_}
            // imgui_{dev_, swchain_, cqueue_}
            {
                //
                dev_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), // TODO add d3d readback type
                                              D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(256), D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                              __uuidof(gtl::d3d::resource::type), expose_as_void_pp(id_readback_));
                // cbuffer_[idx].update() --
                std::cout << "swirl_effect()\n";

                initialize_null_descriptor_srv(dev_, resource_heap_.get_handle(1));
                initialize_null_descriptor_uav(dev_, resource_heap_.get_handle(2));
            }

            void set_mouse_coords(int x, int y) const
            {
                mouse_coord_.set(MAKELPARAM(x, y));
            }

            swirl_effect& operator=(swirl_effect&&)
            {
                std::cout << "swirl_effect operator= called..\n";
                return *this;
            } // TODO throw? assert false?
            swirl_effect(swirl_effect&&) = default;

            ~swirl_effect()
            {
                std::cout << "~swirl_effect()\n";
            }

            void resize(int w, int h, gtl::d3d::command_queue& c)
            {
                viewport_.Width = static_cast<float>(w);
                viewport_.Height = static_cast<float>(h);
                scissor_ = gtl::d3d::raw::ScissorRect{0, 0, w, h};
                // imgui_.resize(w,h,c);
                id_layer_.resize(w, h);
                depth_buffers_.resize(w, h);
            }

            void draw(std::vector<ID3D12CommandList*>& v, int idx, float f, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle, std::atomic<uint32_t>& id_,
                      Eigen::Matrix4f const& camera) const
            {

                update(cbuf_, viewport_.Width / viewport_.Height);
                // cbuffer_[idx].update(reinterpret_cast<const char*>(&cbuf_), sizeof(cbuf_));
                cbuffer_.update(reinterpret_cast<const char*>(&cbuf_), sizeof(cbuf_));

                // copy id layer into id_readback

                uint32_t id_value_{};

                // id_readback_->ReadFromSubresource(&id_value_, 0, 0, 0, &CD3DX12_BOX{0,1});
                void* p{};
                id_readback_->Map(0, &CD3DX12_RANGE{0, 4}, &p);
                memcpy(&id_value_, p, 4);
                id_readback_->Unmap(0, &CD3DX12_RANGE{1, 0}); // end < begin tells the api that nothing was written

                id_.store(id_value_, std::memory_order_relaxed);

                calloc_[idx]->Reset();
                clist_[idx]->Reset(calloc_[idx].get(), pso_.get());
                //
                gtl::d3d::graphics_command_list const& cl = clist_[idx];

                cl->SetGraphicsRootSignature(root_sig_.get());
                auto heaps = {sampler_heap_.get(), resource_heap_.get()};
                cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
                // cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
                cl->SetGraphicsRootConstantBufferView(0, (cbuffer_.resource())->GetGPUVirtualAddress());
                cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
                cl->SetGraphicsRootDescriptorTable(2, resource_heap_->GetGPUDescriptorHandleForHeapStart());

                float blendvalues[]{f, f, f, f};
                cl->OMSetBlendFactor(blendvalues);

                auto viewports = {std::addressof(viewport_)};
                cl->RSSetViewports(static_cast<unsigned>(viewports.size()), *viewports.begin());
                cl->RSSetScissorRects(1, std::addressof(scissor_));
                cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

                CD3DX12_CPU_DESCRIPTOR_HANDLE id_handle{id_layer_.rtv_heap()->GetCPUDescriptorHandleForHeapStart()};
                id_handle.Offset(idx, id_layer_.rtv_heap().increment_value());

                D3D12_CPU_DESCRIPTOR_HANDLE handles[]{rtv_handle, id_handle};

                // cl->OMSetRenderTargets(2, handles, false, nullptr); // not issuing ids with this shader..
                float const clearvalues[]{0.0f, 0.0f, 0.0f, 0.0f};
                cl->ClearRenderTargetView(id_handle, clearvalues, 0, nullptr);

                auto const dbview_ = depth_buffers_.get_handle();
                cl->ClearDepthStencilView(dbview_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

                cl->OMSetRenderTargets(1, &rtv_handle, TRUE, std::addressof(dbview_));
                cl->DrawInstanced(14, 1, 0, 0);
                clist_[idx]->Close();

                gui_rect_clist_[idx]->Reset(calloc_[idx].get(), nullptr); // nullptr or pso_.get() -- advantages?
                gui_rect_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

                // gui_rect_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,camera,handles);
                object_effect_(idx, f, gui_rect_clist_[idx], viewport_, scissor_, camera, handles, std::addressof(dbview_));

                D3D12_TEXTURE_COPY_LOCATION src{id_layer_, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
                D3D12_TEXTURE_COPY_LOCATION dst{id_readback_, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                                                D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0, CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT, 1, 1, 1, 256}}};

                src.SubresourceIndex = idx;
                // dst.PlacedFootprint =
                // D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0,CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT,4,1,0,16}};

                // id layer texture copy..
                gui_rect_clist_[idx]->ResourceBarrier(
                    1, &CD3DX12_RESOURCE_BARRIER::Transition(id_layer_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, idx));

                // gui_rect_clist_[idx]->CopyBufferRegion(id_readback_,0,id_layer_,0,4);

                LPARAM coord_ = mouse_coord_.get();
                int mx = GET_X_LPARAM(coord_);
                int my = GET_Y_LPARAM(coord_);

                static int i = 0; // HACK logging.
                if (++i > 500)
                {
                    i = 0;
                    std::cout << "mouse @ " << mx << "," << my << " :: id == " << id_value_ << "\n";
                    // std::cout << "id value == " << id_value_ << "\n";
                }

                if (viewport_.contains(mx, my))
                {
                    gui_rect_clist_[idx]->CopyTextureRegion(&dst, 0, 0, 0, &src, &CD3DX12_BOX{mx, my, mx + 1, my + 1});
                }

                gui_rect_clist_[idx]->ResourceBarrier(
                    1, &CD3DX12_RESOURCE_BARRIER::Transition(id_layer_, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, idx));

                gui_rect_clist_[idx]->Close();

                //
                v.emplace_back(clist_[idx].get());
                v.emplace_back(gui_rect_clist_[idx].get());

                // v.emplace_back(imgui_clist_[idx].get());
                // v.emplace_back(object_effect_clist_[idx].get());
                // v.emplace_back(id_sampler_clist_[idx].get());

                // return v;
            }
        };
    }
}
} // namespaces
#endif
