#ifndef UTWOWOPQRRR_GTL_SCENES_TWINKLE_EFFECT_TRANSITION_SCENE_H_
#define UTWOWOPQRRR_GTL_SCENES_TWINKLE_EFFECT_TRANSITION_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)

    namespace gtl::scenes::transitions::

    class twinkle_effect;
        + hackish..
-----------------------------------------------------------------------------*/

#include <gtl/font_atlas.h>
#include <gtl/swirl_effect_transition_scene.h>

namespace gtl {
namespace scenes {
    namespace transitions {

        class twinkle_effect {

            constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

            gtl::d3d::device& dev_;
            gtl::d3d::command_queue& cqueue_;
            // gtl::d3d::root_signature& rg_;
            gtl::d3d::swap_chain& swchain_;

            gtl::d3d::vertex_shader vshader_;
            gtl::d3d::pixel_shader pshader_;
            gtl::d3d::root_signature root_sig_;

            std::array<gtl::d3d::resource_descriptor_heap, frame_count> cbheap_;

            cbuffer mutable cbuf_;
            std::array<gtl::d3d::constant_buffer, frame_count> mutable cbuffer_;

            gtl::d3d::pipeline_state_object pso_;

            std::array<gtl::d3d::direct_command_allocator, frame_count> mutable calloc_;
            std::array<gtl::d3d::graphics_command_list, frame_count> mutable clist_;
            std::array<gtl::d3d::graphics_command_list, frame_count> mutable font_clist_;

            gtl::d3d::raw::
                Viewport mutable viewport_; //{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
            gtl::d3d::raw::ScissorRect mutable scissor_; //{0,0,960,540};

            gtl::d3d::raw::
                Viewport mutable text_viewport_; //{0.0f,0.0f,320.0f,240.0f,1.0f,1.0f};

            gtl::d3d::resource_descriptor_heap resource_heap_;
            gtl::d3d::srv texture_;

            gtl::d3d::sampler_descriptor_heap sampler_heap_;
            gtl::d3d::sampler sampler_;

            gtl::d3d::font_atlas font_;
            float mutable font_scale;

        public:
            twinkle_effect(gtl::d3d::device& dev_, gtl::d3d::swap_chain& swchain_,
                gtl::d3d::command_queue& cqueue_) // TODO temporary effect..
                : dev_{ dev_ },
                  cqueue_{ cqueue_ },
                  // root_sig_{root_sig_},
                  swchain_{ swchain_ },
                  vshader_{ L"D:\\Code\\D3D12_migration\\D3D12_"
                            L"migration\\Debug\\x64\\skybox_vs.cso" },
                  pshader_{ L"D:\\Code\\D3D12_migration\\D3D12_"
                            L"migration\\Debug\\x64\\skybox_ps.cso" },
                  root_sig_{ dev_, vshader_ },
                  cbheap_{ { { dev_, 1, gtl::d3d::tags::shader_visible{} },
                      { dev_, 1, gtl::d3d::tags::shader_visible{} },
                      { dev_, 1, gtl::d3d::tags::shader_visible{} } } },
                  cbuf_{ 960.0f / 540.0f },
                  cbuffer_{ { { dev_, cbheap_[0], sizeof(cbuf_) },
                      { dev_, cbheap_[1], sizeof(cbuf_) },
                      { dev_, cbheap_[2], sizeof(cbuf_) } } },
                  pso_{ dev_, root_sig_, vshader_, pshader_ },
                  calloc_{ { { dev_ }, { dev_ }, { dev_ } } },
                  clist_{ { { dev_, calloc_[0], pso_ },
                      { dev_, calloc_[1], pso_ },
                      { dev_, calloc_[2], pso_ } } },
                  font_clist_{
                      { { dev_, calloc_[0] }, { dev_, calloc_[1] }, { dev_, calloc_[2] } }
                  },
                  viewport_{ 0.0f, 0.0f, 960.0f, 540.0f, 0.0f, 1.0f },
                  scissor_{ 0, 0, 960, 540 },
                  text_viewport_{ 0.0f, 0.0f, 320.0f, 240.0f, 1.0f, 1.0f },
                  resource_heap_{ dev_, 2, gtl::d3d::tags::shader_visible{} },
                  texture_{ dev_,
                      { resource_heap_->GetCPUDescriptorHandleForHeapStart() },
                      cqueue_,
                      L"D:\\images\\skyboxes\\Grimmnight.dds" },
                  sampler_heap_{ dev_, 1 },
                  sampler_{ dev_, sampler_heap_->GetCPUDescriptorHandleForHeapStart() },
                  font_{ dev_, cqueue_, root_sig_,
                      // L"D:\\images\\fonts\\liberation\\bold-sdf\\font.fnt",
                      L"D:\\images\\fonts\\depth-field-font72\\font.fnt",
                      gtl::d3d::tags::xml_format{} },
                  font_scale{ 72.0f }
            {
                // cbuffer_[idx].update() --
                std::cout << "twinkle_effect()\n";
            }

            twinkle_effect& operator=(twinkle_effect&&)
            {
                std::cout << "twinkle_effect operator= called..\n";
                return *this;
            } // TODO throw? assert false?
            twinkle_effect(twinkle_effect&&) = default;

            ~twinkle_effect() { std::cout << "~twinkle_effect()\n"; }

            std::vector<ID3D12CommandList*>
            draw(int idx, float f, gtl::d3d::rtv_descriptor_heap& rtv_heap_) const
            {
                update(cbuf_, viewport_.Width / viewport_.Height);
                cbuffer_[idx].update(reinterpret_cast<const char*>(&cbuf_), sizeof(cbuf_));
                //
                std::vector<ID3D12CommandList*> v;
                calloc_[idx]->Reset();
                clist_[idx]->Reset(calloc_[idx].get(), pso_.get());
                //
                gtl::d3d::graphics_command_list const& cl = clist_[idx];

                cl->SetGraphicsRootSignature(root_sig_.get());
                auto heaps = { sampler_heap_.get(), resource_heap_.get() };
                cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
                cl->SetGraphicsRootConstantBufferView(
                    0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
                cl->SetGraphicsRootDescriptorTable(
                    1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
                cl->SetGraphicsRootDescriptorTable(
                    2, resource_heap_->GetGPUDescriptorHandleForHeapStart());

                auto viewports = { std::addressof(viewport_) };
                cl->RSSetViewports(static_cast<unsigned>(viewports.size()),
                    *viewports.begin());
                cl->RSSetScissorRects(1, std::addressof(scissor_));
                cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

                // float blendvalues[]{f,f,f,f};
                // cl->OMSetBlendFactor(blendvalues);

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{
                    rtv_heap_->GetCPUDescriptorHandleForHeapStart()
                };
                rtv_handle.Offset(idx, rtv_heap_.increment_value());

                float clearvalue[]{ 0.0f, 0.0f, 0.0f, 0.0f };

                cl->ClearRenderTargetView(rtv_handle, clearvalue, 1, &scissor_);

                cl->OMSetRenderTargets(1, &rtv_handle, TRUE, nullptr);
                cl->DrawInstanced(14, 1, 0, 0);

                clist_[idx]->Close();
                font_clist_[idx]->Reset(calloc_[idx].get(), nullptr);
                font_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

                float f_scale = font_scale / 72.0f;

                font_(idx, f, font_clist_[idx], text_viewport_, scissor_, f_scale,
                    rtv_handle);
                font_clist_[idx]->Close();

                v.emplace_back(clist_[idx].get());
                v.emplace_back(font_clist_[idx].get());
                return v;
            }

            void draw(std::vector<ID3D12CommandList*>& v, int idx, float f,
                gtl::d3d::rtv_descriptor_heap& rtv_heap_) const
            {
                update(cbuf_, viewport_.Width / viewport_.Height);
                cbuffer_[idx].update(reinterpret_cast<const char*>(&cbuf_), sizeof(cbuf_));
                //

                calloc_[idx]->Reset();
                clist_[idx]->Reset(calloc_[idx].get(), pso_.get());
                //
                gtl::d3d::graphics_command_list const& cl = clist_[idx];

                cl->SetGraphicsRootSignature(root_sig_.get());
                auto heaps = { sampler_heap_.get(), resource_heap_.get() };
                cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());
                cl->SetGraphicsRootConstantBufferView(
                    0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
                cl->SetGraphicsRootDescriptorTable(
                    1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
                cl->SetGraphicsRootDescriptorTable(
                    2, resource_heap_->GetGPUDescriptorHandleForHeapStart());

                auto viewports = { std::addressof(viewport_) };
                cl->RSSetViewports(static_cast<unsigned>(viewports.size()),
                    *viewports.begin());
                cl->RSSetScissorRects(1, std::addressof(scissor_));
                cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

                // float blendvalues[]{f,f,f,f};
                // cl->OMSetBlendFactor(blendvalues);

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{
                    rtv_heap_->GetCPUDescriptorHandleForHeapStart()
                };
                rtv_handle.Offset(idx, rtv_heap_.increment_value());

                float clearvalue[]{ 0.0f, 0.0f, 0.0f, 0.0f };

                cl->ClearRenderTargetView(rtv_handle, clearvalue, 1, &scissor_);

                cl->OMSetRenderTargets(1, &rtv_handle, TRUE, nullptr);
                cl->DrawInstanced(14, 1, 0, 0);

                clist_[idx]->Close();
                font_clist_[idx]->Reset(calloc_[idx].get(), nullptr);
                font_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

                float f_scale = font_scale / 72.0f;

                font_(idx, f, font_clist_[idx], text_viewport_, scissor_, f_scale,
                    rtv_handle);
                font_clist_[idx]->Close();

                v.emplace_back(clist_[idx].get());
                v.emplace_back(font_clist_[idx].get());
            }

            template <typename YieldType>
            gtl::event handle_events(YieldType& yield) const
            {
                namespace ev = gtl::events;
                namespace k = gtl::keyboard;
                int count{};
                while (!same_type(yield().get(), ev::exit_immediately{})) {
                    if (same_type(yield.get(), ev::keydown{})) {

                        switch (boost::get<ev::keydown>(yield.get().value()).key) {
                        case k::Q:
                            std::cout
                                << "twinkle_effect(): q pressed, exiting A from route 0 (none == "
                                << count << ")\n";
                            return gtl::events::exit_state{ 0 };
                            break;
                        case k::K:
                            std::cout << "twinkle_effect(): k pressed, throwing (none == "
                                      << count << ")\n";
                            throw std::runtime_error{ __func__ };
                            break;
                        // case k::R : std::cout << "twinkle_effect() : r pressed, resizing
                        // swapchain..\n";
                        //            swchain_.resize(100,100);
                        //            break;
                        case k::S:
                            text_viewport_.Height++;
                            break;
                        case k::W:
                            text_viewport_.Width++;
                            break;
                        case k::R:
                            text_viewport_.TopLeftY++;
                            break;
                        case k::T:
                            text_viewport_.TopLeftX++;
                            break;
                        case k::A:
                            font_scale += 1.0f;
                            break;
                        case k::D:
                            font_scale -= 1.0f;
                            break;
                        default:
                            std::cout << "twinkle_effect() : unknown key pressed\n";
                        }

                    } else if (same_type(yield.get(), ev::none{})) {
                        count++;
                    }
                }
                return gtl::events::exit_state{ 0 };
            }
        };
    }
}
} // namespaces
#endif
