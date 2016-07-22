#ifndef WYWEFFWFVSF_GTL_D3D_TYPES_H_
#define WYWEFFWFVSF_GTL_D3D_TYPES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::version_12_0 
        
    Various "RAII" versions of D3D types
-----------------------------------------------------------------------------*/

#include <gtl/d3d_include.h>

#include <gtl/tags.h>
#include <gtl/release_ptr.h>
#include <gtl/win_tools.h>

#include <iostream> 
#include <chrono>
#include <array>
#include <vector>
#include <utility>
#include <atomic>
#include <string>
#include <ostream>
#include <cassert>

namespace gtl {        
namespace d3d {    

namespace tags {
    struct shader_visible{};
    struct not_shader_visible{};    
    struct flipmodel_windowed{};

    struct depth_stencil_view{};
    struct cbv_srv_uav{};
}

namespace default = version_12_0;
using namespace default;

namespace version_12_0 {           

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
        device(gtl::tags::uninitialized) noexcept {}
        device(gtl::tags::debug,std::ostream&);
        device(gtl::tags::release);
        device(device&&) = default;
        device& operator=(device&&) = delete;
    };    
                
    class command_queue : public release_ptr<raw::CommandQueue> {        
    public:
        command_queue(device&);
    };

    class rtv_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;
    public:
        rtv_descriptor_heap(device&, unsigned num_descriptors);       
        rtv_descriptor_heap(device&, std::vector<resource>&);       
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }        
    };

    class resource_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;  // TODO currently works with CBV_SRV_UAV types, not DSV (break into distinct types?)
    public:
        resource_descriptor_heap(device&, unsigned num_descriptors, d3d::tags::shader_visible);        
        resource_descriptor_heap(device&, unsigned num_descriptors, d3d::tags::depth_stencil_view);        
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }
        raw::CpuDescriptorHandle get_handle(unsigned n) { 
            assert(n < size_);
            raw::cx::CpuDescriptorHandle handle{get()->GetCPUDescriptorHandleForHeapStart()};
            handle.Offset(n,increment_value());
            return handle;
        }
    };

    class sampler_descriptor_heap : public release_ptr<raw::DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;
    public:
        sampler_descriptor_heap(device&, unsigned num_descriptors);
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }
    };
    
    class swap_chain : public release_ptr<raw::SwapChain> {                
        std::vector<resource> frames_;        
        rtv_descriptor_heap rtv_heap_;
    public:
        swap_chain(HWND, command_queue&, unsigned num_buffers); 
        resource& get_current_resource() { return frames_[get()->GetCurrentBackBufferIndex()]; }
        rtv_descriptor_heap& rtv_heap() { return rtv_heap_; }
        void resize(int,int) { std::cout << "swapchain resizing..\n"; } // TODO implement
        std::pair<unsigned,unsigned> dimensions() const;
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
     
    class fence : public release_ptr<raw::Fence> {                
    public:
        fence(raw::Device&);
        fence(device& dev) : fence(*dev) {}
        fence(fence&& other) = default;
        void synchronized_set(uint64_t new_value, command_queue&);
        void synchronized_increment(command_queue&);
    };
    
    class constant_buffer {
        resource buffer;                
        unsigned char* cbv_data_ptr{};
    public:
        constant_buffer(device&,std::size_t);
        constant_buffer(device&,resource_descriptor_heap&,std::size_t);
        constant_buffer(device&,raw::CpuDescriptorHandle,std::size_t);
        void update(char const*, std::size_t);
        void update(std::pair<char*,size_t>);
        auto& resource() { return buffer; }
        auto const& resource() const { return buffer; }
    };
    
    class vertex_buffer : public release_ptr<raw::Resource> {        
    public:
        vertex_buffer(device&, command_queue&, void* begin, size_t size);
    };    

    class index_buffer : public release_ptr<raw::Resource> {        
    public:
        index_buffer(device&, command_queue&, void* begin, size_t size);
    };    

    class depth_stencil_buffer {                                
        std::vector<resource> buffers_; 
        resource_descriptor_heap buffer_views_;
    public:
        depth_stencil_buffer(swap_chain&);
        auto get_handle(unsigned n) const { 
            assert(n < buffers_.size());
            raw::cx::CpuDescriptorHandle handle{ buffer_views_->GetCPUDescriptorHandleForHeapStart() };
            handle.Offset(n,buffer_views_.increment_value());
            return handle;
        }        
    };        

    class srv : public release_ptr<raw::Resource> {
    public:
        srv(device&,std::vector<raw::CpuDescriptorHandle>,command_queue&,std::wstring);
    };
        
    class sampler : public release_ptr<raw::Resource> {
    public:
        sampler(device&, raw::CpuDescriptorHandle);
        sampler(device&, raw::SamplerDesc const&,raw::CpuDescriptorHandle);
    };

    class rtv_srv_texture2D : public release_ptr<raw::Resource> {
        rtv_descriptor_heap rtv_heap_;
        resource_descriptor_heap srv_heap_;
    public:
        rtv_srv_texture2D(swap_chain&, unsigned num_buffers, d3d::tags::shader_visible);
        rtv_srv_texture2D(swap_chain&, raw::Format, unsigned num_buffers, d3d::tags::shader_visible);

        resource_descriptor_heap& srv_heap() { return srv_heap_; }
        rtv_descriptor_heap& rtv_heap() { return rtv_heap_; }
    };

    class uav_texture2D : public release_ptr<raw::Resource> {
    public:
        uav_texture2D(swap_chain&, raw::CpuDescriptorHandle&);
    };

    using blob = release_ptr<raw::Blob>;


}

}} // namespaces
#endif

