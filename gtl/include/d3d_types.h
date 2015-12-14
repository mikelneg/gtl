#ifndef WYWEFFWFVSF_GTL_D3D_TYPES_H_
#define WYWEFFWFVSF_GTL_D3D_TYPES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0 
    
    useful types
-----------------------------------------------------------------------------*/

#include "d3d_default_implementation.h"

#include <gtl/include/tags.h>
#include <gtl/include/release_ptr.h>
#include <gtl/include/gtl_window.h>
#include <gtl/include/win_tools.h>
#include <chrono>
#include <array>
#include <vector>
#include <utility>
#include <atomic>
#include <string>

namespace gtl {    
namespace d3d {    

namespace _12_0 {    
    
    namespace tags {
        struct shader_visible{};
        struct not_shader_visible{};                
    }
        

    //
    //      device
    //      debug_layer
    //      swap_chain
    //
    //      command_queue
    //      command_allocator
    //      command_list
    //      
    //      descriptor_heap
    //      descriptor
    //
    //      root_signature
    //
    //      pipeline_state_object
    //      v_shader
    //      p_shader
    //      c_shader
    //
    //      resources
    //      
    //      fences
    //
    //
    //
    //          stage a,b,c; present(a,b,c); 
    //
    //
    //
    //    
    //
    //
    //
    //

    class resource : public release_ptr<D3D12Resource> {
    public:        
        resource() = default;
    };
    
    class dxgi_factory : public release_ptr<DXGIFactory> {
    public:
        dxgi_factory();
    };
    
    class device : public release_ptr<D3D12Device> {            
    public:        
        device(gtl::tags::uninitialized) {}
        device(gtl::tags::debug);
        device(gtl::tags::release);
        device(device&&) = default;
        device& operator=(device&&) = delete;
    };    
                
    class command_queue : public release_ptr<D3D12CommandQueue> {        
    public:
        command_queue(device&);
    };

    class rtv_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;
    public:
        rtv_descriptor_heap(device&, unsigned num_descriptors);       
        rtv_descriptor_heap(device&, std::vector<resource>&);       
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }        
    };

    class resource_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;
    public:
        resource_descriptor_heap(device&, unsigned num_descriptors, tags::shader_visible);
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }
    };

    class sampler_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned increment_;
        unsigned const size_;
    public:
        sampler_descriptor_heap(device&, unsigned num_descriptors);
        auto increment_value() const noexcept { return increment_; }
        auto size() const noexcept { return size_; }
    };

    
    class swap_chain : public release_ptr<DXGISwapChain> {                
        std::vector<resource> frames_;        
    public:
        swap_chain(gtl::win::window&, device&, unsigned num_buffers); 
        resource& get_current_resource() { return frames_[get()->GetCurrentBackBufferIndex()]; }                
    };    
    
    class direct_command_allocator : public release_ptr<D3D12CommandAllocator> {
    public:
        direct_command_allocator(device&);
    };
    
    class compute_command_allocator : public release_ptr<D3D12CommandAllocator> {
    public:
        compute_command_allocator(device&);        
    };

    class root_signature : public release_ptr<D3D12RootSignature> {
    public:
        root_signature(device&, release_ptr<D3DBlob> signature);        
    };
    
    class vertex_shader : public release_ptr<D3DBlob> {
    public:
        vertex_shader(std::wstring path);
    };

    class pixel_shader : public release_ptr<D3DBlob> {
    public:
        pixel_shader(std::wstring path);
    };
    
    class compute_shader : public release_ptr<D3DBlob> {
    public:
        compute_shader(std::wstring path);
    };

    class pipeline_state_object : public release_ptr<D3D12PipelineState> {
    public:
        pipeline_state_object(device&, root_signature&, vertex_shader&, pixel_shader&);
        pipeline_state_object(device&, root_signature&, compute_shader&);
    };

    class graphics_command_list : public release_ptr<D3D12GraphicsCommandList> {
    public:
        graphics_command_list(device&, direct_command_allocator&, pipeline_state_object&);
        graphics_command_list(device&, compute_command_allocator&, pipeline_state_object&);       
        graphics_command_list(device&, direct_command_allocator&);       
    };
     
    class fence : public release_ptr<D3D12Fence> {                
    public:
        fence(D3D12Device&);
        fence(device& dev) : fence(*dev) {}
        fence(fence&& other) = default;
        void synchronized_set(uint64_t new_value, command_queue&);
    };

    class constant_buffer {
        resource buffer;                
        unsigned char* cbv_data_ptr{};
    public:
        constant_buffer(device&,resource_descriptor_heap&,std::pair<char*,unsigned>);
        void update(std::pair<char*,unsigned>);
        resource& resource() { return buffer; }
    };

    class srv : public release_ptr<D3D12Resource> {
    public:
        srv(device&,std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>,command_queue&,std::wstring);
    };

    class sampler : public release_ptr<D3D12Resource> {
    public:
        sampler(device&,D3D12_CPU_DESCRIPTOR_HANDLE);
    };

    class rtv_srv_texture2D : public release_ptr<D3D12Resource> {
        rtv_descriptor_heap rtv_heap__;
        resource_descriptor_heap srv_heap_;
    public:
        rtv_srv_texture2D(swap_chain&, unsigned num_buffers, tags::shader_visible);

        resource_descriptor_heap& srv_heap() { return srv_heap_; }
        rtv_descriptor_heap& rtv_heap_() { return rtv_heap__; }
    };

    class uav_texture2D : public release_ptr<D3D12Resource> {
        //resource_descriptor_heap uav_heap_;
        //resource_descriptor_heap srv_heap_;        
        //rtv_descriptor_heap rtv_heap__;        
    public:
        uav_texture2D(swap_chain&, D3D12_CPU_DESCRIPTOR_HANDLE&);

        //resource_descriptor_heap& srv_heap() { return srv_heap_; }
        //resource_descriptor_heap& uav_heap() { return uav_heap_; }
        //rtv_descriptor_heap& rtv_heap_() { return rtv_heap__; }
    };

    //class device : public detail::simple_ptr_base<D3D12Device> {            
    //public:
    //    device();        
    //};    
    //        
    //class command_queue : public detail::simple_ptr_base<D3D12CommandQueue> {        
    //public:
    //    command_queue(device&);
    //};
    //
    //class swap_chain : public detail::simple_ptr_base<DXGISwapChain> {        
    //    unsigned frame_index;
    //public:
    //    swap_chain(gtl::window&, command_queue&);                        
    //};    
    //
    //class rtv_descriptor_heap : public detail::simple_ptr_base<D3D12DescriptorHeap> {
    //    unsigned increment;
    //public:
    //    rtv_descriptor_heap(device&);
    //    auto size() const noexcept { return increment; }
    //};
    //
    //class resource : public detail::simple_ptr_base<D3D12Resource> {
    //public:        
    //    resource() = default;
    //};
    //
    //class swapchain_rtv_heap_ {
    //    std::array<resource,frame_count()> frames;
    //public:
    //    swapchain_rtv_heap_(swap_chain&, device&, rtv_descriptor_heap&);
    //};


}

}} // namespaces
#endif

