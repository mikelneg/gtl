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
#include <gtl/mesh_loader.h>
#include <gtl/physics_simulation.h>

#include <Eigen/Core>

#include <fstream>
#include <string>

#include <algorithm>

namespace gtl {
namespace d3d {

    class object_rendering_scene {

        static constexpr unsigned MAX_ENTITIES = 400;
        static constexpr unsigned MAX_BONES = MAX_ENTITIES * 10; // TODO arbitrary number..

        using vertex_type = Eigen::Vector4f;

        using matrix_type = Eigen::Matrix4f;

        template <typename T>
        using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;

        //gtl::mesh_loader mesh_object_;
        //gtl::mesh_loader mesh_object_two_; // HACK moving to multiple mesh support..
        gtl::mesh_group<aligned_vector<vertex_type_bone>,
                        std::vector<unsigned>,
                        std::string>
            mesh_group_;

        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;

        //aligned_vector<vertex_type_bone> mesh_;
        //aligned_vector<vertex_type_bone> mesh_two_;

        //aligned_vector<matrix_type> bones_;
        //std::vector<unsigned> indices_;

        gtl::d3d::vertex_buffer vbuffer_;
        //gtl::d3d::vertex_buffer vbuffer_two_;
        gtl::d3d::index_buffer index_buffer_;

        gtl::d3d::resource_descriptor_heap ibuffer_descriptors_;

        aligned_vector<InstanceInfo> instance_data_;

        std::array<gtl::d3d::constant_buffer, 3> mutable instance_buffers_;

        //std::array<gtl::d3d::resource_descriptor_heap,3> cbheap_;
        gtl::d3d::resource_descriptor_heap cbheap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable cbuffer_;

        //std::array<gtl::d3d::resource_descriptor_heap,3> bone_heap_;
        gtl::d3d::resource_descriptor_heap bone_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable bone_buffer_;

        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;
        gtl::d3d::srv texture_;

        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::pipeline_state_object pso_;

        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;

        gtl::physics_simulation& physics_;

        //std::vector<EntityInfo> mutable positions_;
        render_data mutable render_data_;

        std::array<bool, 3> mutable position_flags_;

        auto vertex_layout()
        {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                {"VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_BONE_IDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"VERTEX_BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

                {"INSTANCE_INFO", 0, DXGI_FORMAT_R16G16B16A16_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
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
            desc_.VS = {reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize()};
            desc_.PS = {reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize()};
            desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            desc_.RasterizerState.FrontCounterClockwise = true;
            desc_.RasterizerState.DepthClipEnable = true;
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
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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
            aligned_vector<vertex_type> vector_;

            vector_.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back(-1.0f, -1.0f, 1.0f, 1.0f);
            vector_.emplace_back(1.0f, -1.0f, 1.0f, 1.0f);

            return vector_;
        }

        auto raw_mesh(std::wstring filename)
        {
            aligned_vector<vertex_type> vector_;
            std::ifstream file_{filename};
            std::string line_;
            while (std::getline(file_, line_))
            {
                std::istringstream input_{line_};
                float x, y, z;
                while (input_ >> x >> y >> z)
                {
                    vector_.emplace_back(x, y, z, 1.0f);
                }
            }

            return vector_;
        }

    public:
        object_rendering_scene() = default;
        object_rendering_scene(object_rendering_scene&&) = default;
        object_rendering_scene& operator=(object_rendering_scene&&) = default;

        object_rendering_scene(gtl::d3d::device& dev, gtl::d3d::command_queue& cqueue,
                               gtl::d3d::root_signature& rsig, gtl::physics_simulation& physics_)
            : layout_(vertex_layout()), mesh_group_{[]() {
                  decltype(mesh_group_) ret;
                  {
                      gtl::mesh_loader m{"D:\\meshes\\deformed_armature.fbx", gtl::tags::fbx_format{}};
                      ret.add_mesh("deformed_armature", m.bone_vertices(), m.indices(), m.bone_count());
                  }
                  {
                      gtl::mesh_loader m{"D:\\meshes\\monkey.fbx", gtl::tags::fbx_format{}};
                      ret.add_mesh("monkey_armature", m.bone_vertices(), m.indices(), m.bone_count());
                  }
                  return ret;
              }()},
              //mesh_object_("D:\\meshes\\deformed_armature.fbx",gtl::tags::fbx_format{}),
              //mesh_object_two_("D:\\meshes\\deformed_armature.fbx",gtl::tags::fbx_format{}),
              //mesh_(mesh_object_.bone_vertices()),
              //mesh_two_(mesh_object_two_.bone_vertices()),
              //indices_(mesh_object_.indices()),//[&](){ auto vec = mesh_object_.indices();
              //auto v = mesh_object_two_.indices();
              //vec.insert(begin(v),end(v),end(vec)); return vec; }()),
              vbuffer_{dev, cqueue, static_cast<void*>(mesh_group_.vertex_data()), mesh_group_.vertex_count() * sizeof(vertex_type_bone)},
              //vbuffer_two_{dev,cqueue,mesh__.data(),mesh_two_.size() * sizeof(vertex_type_bone)},
              index_buffer_{dev, cqueue, static_cast<void*>(mesh_group_.index_data()), mesh_group_.index_count() * sizeof(uint32_t)},
              ibuffer_descriptors_{dev, 3, gtl::d3d::tags::shader_visible{}},
              instance_buffers_{{{dev, ibuffer_descriptors_.get_handle(0), MAX_ENTITIES * sizeof(EntityInfo)}, {dev, ibuffer_descriptors_.get_handle(1), MAX_ENTITIES * sizeof(EntityInfo)}, {dev, ibuffer_descriptors_.get_handle(2), MAX_ENTITIES * sizeof(EntityInfo)}}},

              cbheap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              cbuffer_{{{dev, cbheap_.get_handle(0), sizeof(gtl::camera)}, {dev, cbheap_.get_handle(1), sizeof(gtl::camera)}, {dev, cbheap_.get_handle(2), sizeof(gtl::camera)}}},
              bone_heap_{dev, 3, gtl::d3d::tags::shader_visible{}},
              bone_buffer_{{{dev, bone_heap_.get_handle(0), MAX_BONES * sizeof(Eigen::Matrix4f), gtl::d3d::tags::shader_view{}}, {dev, bone_heap_.get_handle(1), MAX_BONES * sizeof(Eigen::Matrix4f), gtl::d3d::tags::shader_view{}}, {dev, bone_heap_.get_handle(2), MAX_BONES * sizeof(Eigen::Matrix4f), gtl::d3d::tags::shader_view{}}}},
              texture_descriptor_heap_{dev, 1, gtl::d3d::tags::shader_visible{}},
              texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue, L"D:\\images\\palettes\\greenish_palette.dds"},
              vshader_{L"object_rendering_scene_vs.cso"},
              pshader_{L"object_rendering_scene_ps.cso"},
              pso_{dev, pso_desc(dev, rsig, vshader_, pshader_)},
              sampler_heap_{dev, 1},
              sampler_{dev, sampler_desc(), sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
              physics_{physics_},
              render_data_{}
        {

            auto& positions_ = render_data_.entities_;

            instance_buffers_[0].update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(EntityInfo));
            instance_buffers_[1].update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(EntityInfo));
            instance_buffers_[2].update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(EntityInfo));

            position_flags_[0] = false; // dirty flags.. fix these..
            position_flags_[1] = false;
            position_flags_[2] = false;
        }

        void update_instance_buffer(unsigned idx) const
        {
            if (physics_.extract_render_data(render_data_))
            {
                for (auto&& e : position_flags_)
                    e = true; // set dirty
                position_flags_[idx] = false;
                //construct_vertices(positions_);
                auto& positions_ = render_data_.entities_;
                instance_buffers_[idx].update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(EntityInfo));
                auto& bones_ = render_data_.bones_;
                bone_buffer_[idx].update(reinterpret_cast<char*>(bones_.data()), bones_.size() * sizeof(Eigen::Matrix4f));
            }
            else
            {
                if (position_flags_[idx])
                {
                    position_flags_[idx] = false;
                    //construct_vertices(positions_);
                    auto& positions_ = render_data_.entities_;

                    // sort..
                    std::sort(begin(positions_), end(positions_),
                              [](auto& lhs, auto& rhs) {
                                  return lhs.mesh_id() < rhs.mesh_id();
                              });
                    //

                    instance_buffers_[idx].update(reinterpret_cast<char*>(positions_.data()), positions_.size() * sizeof(EntityInfo));
                    auto& bones_ = render_data_.bones_;
                    bone_buffer_[idx].update(reinterpret_cast<char*>(bones_.data()), bones_.size() * sizeof(Eigen::Matrix4f));
                }
            }
        }

        void operator()(unsigned idx, float, gtl::d3d::graphics_command_list& cl,
                        gtl::d3d::raw::Viewport const& viewport,
                        gtl::d3d::raw::ScissorRect const& scissor,
                        Eigen::Matrix4f const& camera,
                        D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handle,
                        D3D12_CPU_DESCRIPTOR_HANDLE const* dbv_handle) const
        {
            //update_camera_buffers(camera);
            cbuffer_[idx].update(reinterpret_cast<const char*>(&camera), sizeof(gtl::camera));
            update_instance_buffer(idx);
            cl->SetPipelineState(pso_.get());
            auto heaps = {sampler_heap_.get(), texture_descriptor_heap_.get()};
            cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
            //cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            //cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());

            cl->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport), 0);

            //cl->SetGraphicsRootConstantBufferView(4, (bone_buffer_[idx].resource())->GetGPUVirtualAddress());
            cl->SetGraphicsRootShaderResourceView(4, (bone_buffer_[idx].resource())->GetGPUVirtualAddress());

            //auto& positions_ = render_data_.entities_;
            //D3D12_VERTEX_BUFFER_VIEW iaviews_[] = {
            //        {vbuffer_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_.size() * sizeof(vertex_type_bone)), sizeof(vertex_type_bone)},
            //        {instance_buffers_[idx].resource()->GetGPUVirtualAddress(), static_cast<unsigned>(positions_.size() * sizeof(EntityInfo)),sizeof(EntityInfo)}};
            //
            //D3D12_VERTEX_BUFFER_VIEW iaviews_two_[] = {
            //        {vbuffer_two_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_two_.size() * sizeof(vertex_type_bone)), sizeof(vertex_type_bone)},
            //        {instance_buffers_[idx].resource()->GetGPUVirtualAddress(), static_cast<unsigned>(positions_.size() * sizeof(EntityInfo)),sizeof(EntityInfo)}};

            //D3D12_INDEX_BUFFER_VIEW ibv{
            //    index_buffer_->GetGPUVirtualAddress(),
            //    static_cast<unsigned>(indices_.size()) * sizeof(unsigned),
            //    DXGI_FORMAT_R32_UINT};

            //cl->IASetIndexBuffer(&ibv);

            auto viewports = {std::addressof(viewport)};
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()), *viewports.begin());

            //float blendvalues[]{f,f,f,f};
            //cl->OMSetBlendFactor(blendvalues);

            cl->RSSetScissorRects(1, &scissor);
            cl->OMSetRenderTargets(2, rtv_handle, false, dbv_handle);

            //std::vector<std::pair<unsigned,unsigned>> draw_info_;

            std::array<uint16_t, 2> counts{};

            auto& positions_ = render_data_.entities_;
            for (auto&& e : positions_)
            {
                counts[e.mesh_id()]++;
            }

            //std::equal_range

            unsigned counter{};
            unsigned starter{};

            D3D12_VERTEX_BUFFER_VIEW iaviews_[] = {
                {vbuffer_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_group_.vertex_count() * sizeof(vertex_type_bone)), sizeof(vertex_type_bone)},
                {instance_buffers_[idx].resource()->GetGPUVirtualAddress(), static_cast<unsigned>(positions_.size() * sizeof(EntityInfo)), sizeof(EntityInfo)}};

            D3D12_INDEX_BUFFER_VIEW ibv{
                index_buffer_->GetGPUVirtualAddress(),
                static_cast<unsigned>(mesh_group_.index_count()) * sizeof(uint32_t),
                DXGI_FORMAT_R32_UINT};

            cl->IASetIndexBuffer(&ibv);
            cl->IASetVertexBuffers(0, 2, iaviews_);

            mesh_group_.apply(
                [&](auto const& key, auto const& voff, auto const& ioff, auto const& vcount, auto const& icount, auto const& bcount) {
                    auto& positions_ = render_data_.entities_;

                    uint32_t bone_count = static_cast<uint32_t>(bcount);
                    cl->SetGraphicsRoot32BitConstants(5, 1, std::addressof(bone_count), 0);

                    cl->DrawIndexedInstanced(static_cast<unsigned>(icount), // indexcountperinstance
                                             counts[counter],
                                             //static_cast<unsigned>(positions_.size()), // instancecount
                                             static_cast<unsigned>(ioff), // start index location
                                             static_cast<unsigned>(voff), // base vertex location
                                             starter);                    // start instance location

                    starter += counts[counter];
                    counter++;
                }

                );

            //for (auto b = begin(positions_), e = end(positions_); b != e; ++b) {
            //    draw_info_.emplace_back((*b).entity_data_
            //}

            //cl->DrawInstanced(static_cast<unsigned>(mesh_.size()), static_cast<unsigned>(positions_.size()), 0, 0);
            // cl->DrawIndexedInstanced(static_cast<unsigned>(indices_.size()), // indexcountperinstance
            //                          static_cast<unsigned>(positions_.size()), // instancecount
            //                          0,             // start index location
            //                          0,             // base vertex location
            //                          0);            // start instance location
        }
    };
}
} // namespaces
#endif
