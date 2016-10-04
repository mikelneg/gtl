/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef WYWEFFWFVSF_GTL_D3D_TYPES_H_
#define WYWEFFWFVSF_GTL_D3D_TYPES_H_

/*-------------------------------------------------------------

namespace gtl::d3d
        + collection of RAII conforming versions of standard d3d12 types
        + currently hackish; still tinkering with interfaces

---------------------------------------------------------------*/

#include <gtl/d3d_tags.h>
#include <gtl/d3d_common.h>

#include <gtl/intrusive_ptr.h>
#include <gtl/tags.h>
#include <gtl/win_tools.h>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace gtl {
namespace d3d {

    namespace detail {
        struct releaser {
            template <typename T>
            void operator()(T* t) const noexcept
            {
                if (t)
                    t->Release();
            }
        };
    }

    template <typename T>
    using release_ptr = gtl::intrusive_ptr<T, detail::releaser>;

    namespace default =
        version_12_0; // using namespace default; appears below

        namespace version_12_0 {

            class command_queue;
            class device;
            class fence;
            class resource;
            class swap_chain;
            class dxgi_factory;

            class rtv_descriptor_heap;
            class resource_descriptor_heap;
            class sampler_descriptor_heap;

            class fence : public release_ptr<raw::Fence> {
            public:
                fence(device&);
                fence(fence&& other) = default;
                void synchronized_set(uint64_t new_value, command_queue&);
                void synchronized_increment(command_queue&);
            };

            class resource : public release_ptr<raw::Resource> {
            public:
                resource() = default;
                resource(resource&&) = default;
            };

            class dxgi_factory : public release_ptr<raw::Factory> {
            public:
                dxgi_factory();
            };

            class device : public release_ptr<raw::Device> {
            public:
                device(gtl::tags::uninitialized) noexcept
                {
                }
                device(gtl::tags::debug, std::ostream&);
                device(gtl::tags::release);
                device(device&&) = default;
                device& operator=(device&&) = delete;
            };

            class command_queue : public release_ptr<raw::CommandQueue> {
                fence fence_; //  internal fence used to synchronize
            public:
                command_queue(device&);
                void synchronize();
            };

            class rtv_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
                unsigned increment_;
                unsigned size_;

            public:
                rtv_descriptor_heap(device&, unsigned num_descriptors);
                // rtv_descriptor_heap(device&, std::vector<resource>&);
                auto increment_value() const noexcept
                {
                    return increment_;
                }
                auto size() const noexcept
                {
                    return size_;
                }
                raw::CpuDescriptorHandle get_handle(unsigned n)
                {
                    assert(n < size_);
                    raw::cx::CpuDescriptorHandle handle{get()->GetCPUDescriptorHandleForHeapStart()};
                    handle.Offset(n, increment_value());
                    return handle;
                }
            };

            class resource_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
                unsigned increment_;
                unsigned const size_; // TODO currently works with CBV_SRV_UAV types, not DSV (break into distinct types?)
            public:
                resource_descriptor_heap(device&, unsigned num_descriptors, d3d::tags::shader_visible);
                resource_descriptor_heap(device&, unsigned num_descriptors, d3d::tags::depth_stencil_view);
                auto increment_value() const noexcept
                {
                    return increment_;
                }
                auto size() const noexcept
                {
                    return size_;
                }
                raw::CpuDescriptorHandle get_handle(unsigned n)
                {
                    assert(n < size_);
                    raw::cx::CpuDescriptorHandle handle{get()->GetCPUDescriptorHandleForHeapStart()};
                    handle.Offset(n, increment_value());
                    return handle;
                }
            };

            class sampler_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
                unsigned increment_;
                unsigned const size_;

            public:
                sampler_descriptor_heap(device&, unsigned num_descriptors);
                auto increment_value() const noexcept
                {
                    return increment_;
                }
                auto size() const noexcept
                {
                    return size_;
                }
            };

            class swap_chain : public release_ptr<raw::SwapChain> {
                std::vector<resource> frames_;
                rtv_descriptor_heap rtv_heap_;
                raw::Format format_;

            public:
                swap_chain(HWND, command_queue&, unsigned num_buffers);
                resource& get_current_resource()
                {
                    return frames_[get()->GetCurrentBackBufferIndex()];
                }
                rtv_descriptor_heap& rtv_heap()
                {
                    return rtv_heap_;
                }
                D3D12_CPU_DESCRIPTOR_HANDLE get_current_handle()
                {
                    return rtv_heap_.get_handle(get()->GetCurrentBackBufferIndex());
                }
                std::pair<int, int> resize(int, int); // { std::cout << "swapchain resizing..\n"; } // TODO implement
                std::pair<unsigned, unsigned> dimensions() const;
                raw::Viewport viewport() const;
                unsigned frame_count() const;
            };

            class direct_command_allocator : public release_ptr<raw::CommandAllocator> {
            public:
                direct_command_allocator(device&);
            };

            class compute_command_allocator : public release_ptr<raw::CommandAllocator> {
            public:
                compute_command_allocator(device&);
            };

            class vertex_shader : public release_ptr<raw::Blob> {
            public:
                vertex_shader(std::wstring path);
            };

            class geometry_shader : public release_ptr<raw::Blob> {
            public:
                geometry_shader(std::wstring path);
            };

            class pixel_shader : public release_ptr<raw::Blob> {
            public:
                pixel_shader(std::wstring path);
            };

            class compute_shader : public release_ptr<raw::Blob> {
            public:
                compute_shader(std::wstring path);
            };

            class root_signature : public release_ptr<raw::RootSignature> {
            public:
                root_signature(device&, release_ptr<raw::Blob> signature);
                root_signature(device&, vertex_shader&);
            };

            class pipeline_state_object : public release_ptr<raw::PipelineState> {
            public:
                pipeline_state_object(device&, root_signature&, vertex_shader&, pixel_shader&);
                pipeline_state_object(device&, root_signature&, compute_shader&);
                pipeline_state_object(device&, raw::GraphicsPipelineStateDesc const&);
            };

            class graphics_command_list : public release_ptr<raw::GraphicsCommandList> {
            public:
                graphics_command_list(device&, direct_command_allocator&, pipeline_state_object&);
                graphics_command_list(device&, compute_command_allocator&, pipeline_state_object&);
                graphics_command_list(device&, direct_command_allocator&);
            };

            class viewport : public raw::Viewport {
            public:
                viewport() = default;
                viewport(viewport const&) = default;
                viewport(raw::Viewport const& v) noexcept : raw::Viewport(v)
                {
                }

                template <typename P>
                bool contains(P const& point) const noexcept
                {
                    auto& x = point.first;
                    auto& y = point.second;
                    return (x >= TopLeftX && y >= TopLeftY && x < (TopLeftX + Width) && y < (TopLeftY + Height));
                }

                template <typename T>
                bool contains(T x, T y) const noexcept
                {
                    return (x >= TopLeftX && y >= TopLeftY && x < (TopLeftX + Width) && y < (TopLeftY + Height));
                }
            };

            class constant_buffer {
                resource buffer;
                unsigned char* cbv_data_ptr{};

            public:
                constant_buffer(device&, std::size_t);
                constant_buffer(device&, resource_descriptor_heap&, std::size_t);
                constant_buffer(device&, raw::CpuDescriptorHandle, std::size_t);
                constant_buffer(device&, raw::CpuDescriptorHandle, std::size_t, d3d::tags::shader_view);
                void update(char const*, std::size_t);
                void update(std::pair<char*, size_t>);
                void update(char const*, std::size_t count, std::size_t offset);
                auto& resource()
                {
                    return buffer;
                }
                auto const& resource() const
                {
                    return buffer;
                }
            };

            class vertex_buffer : public release_ptr<raw::Resource> {
            public:
                vertex_buffer(device&, command_queue&, void* begin, size_t size);
                void update(char const*, std::size_t);
            };

            class index_buffer : public release_ptr<raw::Resource> {
            public:
                index_buffer(device&, command_queue&, void* begin, size_t size);
                void update(char const*, std::size_t);
            };

            class depth_stencil_buffer : public release_ptr<raw::Resource> {
                resource_descriptor_heap buffer_view_;

            public:
                depth_stencil_buffer(swap_chain&);
                auto get_handle() const
                {
                    return buffer_view_->GetCPUDescriptorHandleForHeapStart();
                }
                void resize(int w, int h);
            };

            class srv : public release_ptr<raw::Resource> {
            public:
                srv& operator=(srv&& o)
                {
                    this->reset(o.release());
                    return *this;
                }
                srv(device&, std::vector<raw::CpuDescriptorHandle>, command_queue&, std::wstring);
                srv(device&, std::vector<raw::CpuDescriptorHandle>, command_queue&, std::tuple<std::vector<uint32_t>, unsigned, unsigned>);
            };

            class sampler : public release_ptr<raw::Resource> {
            public:
                sampler(device&, raw::CpuDescriptorHandle);
                sampler(device&, raw::SamplerDesc const&, raw::CpuDescriptorHandle);
            };

            class rtv_srv_texture2D : public release_ptr<raw::Resource> {
                rtv_descriptor_heap rtv_heap_;
                resource_descriptor_heap srv_heap_;

            public:
                rtv_srv_texture2D& operator=(rtv_srv_texture2D&& o)
                {
                    rtv_heap_.reset(o.rtv_heap_.release());
                    srv_heap_.reset(o.srv_heap_.release());
                    this->reset(o.release());
                    return *this;
                }
                rtv_srv_texture2D(swap_chain&, unsigned num_buffers, d3d::tags::shader_visible);
                rtv_srv_texture2D(swap_chain&, raw::Format, unsigned num_buffers, d3d::tags::shader_visible);

                void resize(int w, int h);

                resource_descriptor_heap& srv_heap()
                {
                    return srv_heap_;
                }
                rtv_descriptor_heap& rtv_heap()
                {
                    return rtv_heap_;
                }
            };

            class uav_texture2D : public release_ptr<raw::Resource> {
            public:
                uav_texture2D(swap_chain&, raw::CpuDescriptorHandle&);
            };

            using blob = release_ptr<raw::Blob>;
        }

        using namespace default;
}
} // namespaces
#endif
