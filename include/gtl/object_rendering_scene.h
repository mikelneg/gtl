/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UWIOTOWTWAASFSAXX_GTL_OBJECT_RENDERING_SCENE_H_
#define UWIOTOWTWAASFSAXX_GTL_OBJECT_RENDERING_SCENE_H_

#include <vector>

#include <gtl/d3d_types.h>

#include <gtl/camera.h>
#include <gtl/mesh_group.h>
#include <gtl/common_mesh.h>

#include <gtl/physics/simulation_interface.h>
#include <gtl/physics/common_types.h>

#include <boost/container/flat_map.hpp>

#include <Eigen/Core>

#include <fstream>
#include <string>

#include <algorithm>

namespace gtl {
namespace d3d {

    class object_rendering_scene {

        static constexpr unsigned MAX_ENTITIES = 2000;
        static constexpr unsigned MAX_BONES = MAX_ENTITIES * 4; // TODO arbitrary number..

        using vertex_type = Eigen::Vector4f;

        using matrix_type = Eigen::Matrix4f;

        template <typename T>
        using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;

        // gtl::mesh_group<renderer_vertex_type, uint32_t, std::string> mesh_group_;

        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;

        gtl::mesh_group<renderer_vertex_type, uint32_t, std::string>& mesh_group_;

        gtl::d3d::vertex_buffer vbuffer_;
        gtl::d3d::index_buffer index_buffer_;
        gtl::d3d::resource_descriptor_heap ibuffer_descriptors_;
        std::vector<gtl::entity::render_data> instance_data_;

        gtl::d3d::constant_buffer mutable instance_buffers_;
        gtl::d3d::resource_descriptor_heap cbheap_;

        gtl::d3d::constant_buffer mutable cbuffer_;
        gtl::d3d::resource_descriptor_heap bone_heap_;

        gtl::d3d::constant_buffer mutable bone_buffer_;
        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;

        gtl::d3d::srv texture_;

        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;
        gtl::d3d::pipeline_state_object pso_;

        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;
        gtl::physics::simulation& physics_;
        gtl::physics::simulation_render_data mutable render_data_;

        auto vertex_layout()
        {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                {"VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_BONE_IDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"INSTANCE_INFO", 0, DXGI_FORMAT_R16G16B16A16_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}};
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
            desc_.RasterizerState.DepthClipEnable = true;
            desc_.RasterizerState.AntialiasedLineEnable = true;

            desc_.InputLayout.pInputElementDescs = &layout_[0];
            desc_.InputLayout.NumElements = static_cast<unsigned>(layout_.size());

            D3D12_BLEND_DESC blend_desc_ = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            blend_desc_.RenderTarget[0].BlendEnable = false; // HACK testing..
            blend_desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blend_desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blend_desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blend_desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blend_desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blend_desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blend_desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            blend_desc_.RenderTarget[1].BlendEnable = false;
            // blend_desc_.RenderTarget[1].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            // blend_desc_.RenderTarget[1].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            // blend_desc_.RenderTarget[1].BlendOp = D3D12_BLEND_OP_ADD;
            // blend_desc_.RenderTarget[1].SrcBlendAlpha = D3D12_BLEND_ZERO;
            // blend_desc_.RenderTarget[1].DestBlendAlpha = D3D12_BLEND_ONE;
            // blend_desc_.RenderTarget[1].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            // blend_desc_.RenderTarget[1].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            desc_.BlendState = blend_desc_;

            desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            desc_.DepthStencilState.DepthEnable = TRUE;
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
            sampler_.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            sampler_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.MaxAnisotropy = 4;
            sampler_.BorderColor[3] = 0.0f; // no alpha
            sampler_.ComparisonFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
            return sampler_;
        }

        auto dummy_mesh()
        {
            std::vector<vertex_type> vector_;

            vector_.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back(-1.0f, -1.0f, 1.0f, 1.0f);
            vector_.emplace_back(1.0f, -1.0f, 1.0f, 1.0f);

            return vector_;
        }

    public:
        object_rendering_scene() = default;
        object_rendering_scene(object_rendering_scene&&) = default;
        object_rendering_scene& operator=(object_rendering_scene&&) = default;

        object_rendering_scene(gtl::d3d::device& dev, gtl::d3d::command_queue& cqueue, gtl::d3d::root_signature& rsig, gtl::physics::simulation& physics_,
                               gtl::mesh_group<renderer_vertex_type, uint32_t, std::string>& mesh_group_)
            : mesh_group_{mesh_group_},
              layout_(vertex_layout()),
              // mesh_group_{[]()
              //  {
              //    decltype(mesh_group_) ret;
              //    { gtl::mesh::mesh_loader m{"data\\meshes\\cone.fbx", gtl::tags::mesh_format_fbx{}};
              //      ret.add_mesh("cone", m.assembled_vertices(), m.indices(), m.bone_count());
              //    }
              //    { gtl::mesh::mesh_loader m{"data\\meshes\\eyeball.fbx", gtl::tags::mesh_format_fbx{}};
              //      ret.add_mesh("eyeball", m.assembled_vertices(), m.indices(), m.bone_count());
              //    }
              //    { gtl::mesh::mesh_loader m{"data\\meshes\\correct_armature.fbx", gtl::tags::mesh_format_fbx{}};
              //      ret.add_mesh("rectangle", m.assembled_vertices(), m.indices(), m.bone_count());
              //    }
              //    return ret;
              //  }()},
              vbuffer_{cqueue, mesh_group_.vertex_data(), mesh_group_.vertex_data_size(), sizeof(renderer_vertex_type)},
              index_buffer_{cqueue, mesh_group_.index_data(), mesh_group_.index_data_size(), DXGI_FORMAT_R32_UINT},
              ibuffer_descriptors_{dev, 3, gtl::d3d::tags::shader_visible{}},
              instance_buffers_{dev, ibuffer_descriptors_.get_handle(0), MAX_ENTITIES * sizeof(gtl::entity::render_data)},
              cbheap_{dev, 1, gtl::d3d::tags::shader_visible{}},
              cbuffer_{dev, cbheap_.get_handle(0), sizeof(Eigen::Matrix4f) * 2},
              bone_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              bone_buffer_{dev, bone_heap_.get_handle(0), MAX_BONES * sizeof(Eigen::Matrix4f), gtl::d3d::tags::shader_view{}},
              texture_descriptor_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue, L"data\\images\\rust.dds"}, // palettes\\greenish_palette.dds"},
              vshader_{L"object_rendering_scene_vs.cso"},
              pshader_{L"object_rendering_scene_ps.cso"},
              pso_{dev, pso_desc(dev, rsig, vshader_, pshader_)},
              sampler_heap_{dev, 1},
              sampler_{dev, sampler_desc(), sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
              physics_{physics_},
              render_data_{}
        {

            auto& positions_ = render_data_.entities_;

            instance_buffers_.update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(gtl::entity::render_data));

            initialize_null_descriptor_srv(dev, texture_descriptor_heap_.get_handle(1));
            initialize_null_descriptor_uav(dev, texture_descriptor_heap_.get_handle(2));
        }

        void update_instance_buffer() const
        {
            if (physics_.extract_render_data(render_data_))
            {
                auto& positions_ = render_data_.entities_;

                std::sort(begin(positions_), end(positions_),
                          [](gtl::entity::render_data const& a, gtl::entity::render_data const& b) { return a.mesh_id() < b.mesh_id(); });

                instance_buffers_.update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(gtl::entity::render_data));
                auto& control_points_ = render_data_.control_points_;
                bone_buffer_.update(reinterpret_cast<char*>(control_points_.data()), control_points_.size() * sizeof(Eigen::Matrix4f));
            }
        }

        void operator()(unsigned idx, float, gtl::d3d::graphics_command_list& cl, gtl::d3d::raw::Viewport const& viewport,
                        gtl::d3d::raw::ScissorRect const& scissor, Eigen::Matrix4f const& camera, Eigen::Matrix4f const& proj,
                        D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handle, D3D12_CPU_DESCRIPTOR_HANDLE const* dbv_handle) const
        {
            cbuffer_.update(reinterpret_cast<const char*>(camera.data()), sizeof(camera));
            cbuffer_.update(reinterpret_cast<const char*>(proj.data()), sizeof(proj), sizeof(camera));

            auto& positions_ = render_data_.entities_;

            update_instance_buffer();

            cl->SetPipelineState(pso_.get());

            auto heaps = {sampler_heap_.get(), texture_descriptor_heap_.get()};

            cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_.resource())->GetGPUVirtualAddress());
            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport), 0);
            cl->SetGraphicsRootShaderResourceView(4, (bone_buffer_.resource())->GetGPUVirtualAddress());

            auto viewports = {std::addressof(viewport)};
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()), *viewports.begin());

            // float blendvalues[]{f,f,f,f};
            // cl->OMSetBlendFactor(blendvalues);

            cl->RSSetScissorRects(1, &scissor);
            cl->OMSetRenderTargets(2, rtv_handle, false, dbv_handle);

            boost::container::flat_map<uint16_t, UINT> counts;

            for (auto&& e : positions_)
            {
                counts[e.mesh_id()]++;
            }

            unsigned counter{};
            unsigned starter{};

            D3D12_VERTEX_BUFFER_VIEW iaviews_[]
                = {vbuffer_.view(), //{vbuffer_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_group_.vertex_data_size()), sizeof(renderer_vertex_type)},
                   {instance_buffers_.resource()->GetGPUVirtualAddress(), static_cast<unsigned>(positions_.size() * sizeof(gtl::entity::render_data)),
                    sizeof(gtl::entity::render_data)}};

            // D3D12_INDEX_BUFFER_VIEW ibv{index_buffer_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_group_.index_data_size()), DXGI_FORMAT_R32_UINT};

            cl->IASetIndexBuffer(&index_buffer_.view());
            cl->IASetVertexBuffers(0, 2, iaviews_);

            mesh_group_.apply([&](auto const& mesh) {
                cl->SetGraphicsRoot32BitConstant(5, static_cast<uint32_t>(mesh.weights_per_vertex), 0);

                cl->DrawIndexedInstanced(static_cast<unsigned>(mesh.index_count), // indexcountperinstance
                                         counts[counter],
                                         static_cast<unsigned>(mesh.index_offset),  // start index location
                                         static_cast<unsigned>(mesh.vertex_offset), // base vertex location
                                         starter);                                  // start instance location

                starter += counts[counter];
                counter++;
            });
        }
    };
}
} // namespaces
#endif
