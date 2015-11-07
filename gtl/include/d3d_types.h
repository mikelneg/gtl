#ifndef WYWEFFWFVSF_GTL_D3D_TYPES_H_
#define WYWEFFWFVSF_GTL_D3D_TYPES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d::_12_0 
    
    useful types
-----------------------------------------------------------------------------*/

#include "d3d_default_implementation.h"

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
    
    /////////////////////////////
    namespace detail {  
        template <typename T>
        class simple_ptr_base {                                
            release_ptr<T> ptr;
        protected:
            release_ptr<T>& expose_ptr() noexcept { return ptr; }
        public:
            using resource_type = T;
            T& get() const noexcept { return *ptr; }
            T* operator->() const noexcept { return ptr.get(); }
        };
    }
    /////////////////////////////
    
    class debug_layer : public release_ptr<D3D12Debug> {
    public:
        debug_layer();
    };

    class device : public release_ptr<D3D12Device> {            
    public:
        device();
        device(device&&) = default;
    };    
                
    class cpu_handle {
        CpuDescriptorHandle handle_;
    public:
        cpu_handle(CpuDescriptorHandle h) : handle_(h) {}
        CpuDescriptorHandle& get() { return handle_; }
    };

    class command_queue : public release_ptr<D3D12CommandQueue> {        
    public:
        command_queue(device&);
    };

    class rtv_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned int increment_;
    public:
        rtv_descriptor_heap(device&, unsigned num_descriptors, tags::not_shader_visible);       
        auto increment_value() const noexcept { return increment_; }
    };

    class resource_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned int increment_;
    public:
        resource_descriptor_heap(device&, unsigned num_descriptors, tags::shader_visible);
        auto increment_value() const noexcept { return increment_; }
    };

    class sampler_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        unsigned int increment_;
    public:
        sampler_descriptor_heap(device&);
        auto increment_value() const noexcept { return increment_; }
    };

    class resource : public release_ptr<D3D12Resource> {
    public:        
        resource() = default;
    };
    
    class swap_chain : public release_ptr<DXGISwapChain> {                
        std::vector<resource> frame_resources;
        rtv_descriptor_heap rtv_heap;                

    public:
        swap_chain(gtl::window&, command_queue&, unsigned num_buffers);        
        resource& get_current_resource() { return frame_resources[get()->GetCurrentBackBufferIndex()]; }
        HANDLE get_waitable_object() { return get()->GetFrameLatencyWaitableObject(); }
        rtv_descriptor_heap& get_rtv_heap() { return rtv_heap; }        
        auto get_current_frame_index() const { return get()->GetCurrentBackBufferIndex(); }   
        auto get_handle_to_current_resource() { 
            CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtv_heap->GetCPUDescriptorHandleForHeapStart(), 
                                                 get_current_frame_index(), 
                                                 rtv_heap.increment_value());
            return handle;
        }
    };    

    //class swapchain_rtv_heap {
    //    std::vector<resource> frames;
    //public:
    //    swapchain_rtv_heap(swap_chain&, device&, rtv_descriptor_heap&);
    //    decltype(frames) const& get_frames() const { return frames; }
    //};

    class direct_command_allocator : public release_ptr<D3D12CommandAllocator> {
    public:
        direct_command_allocator(device&);
        //~direct_command_allocator();
    };
    
    class compute_command_allocator : public release_ptr<D3D12CommandAllocator> {
    public:
        compute_command_allocator(device&);
        //~compute_command_allocator();
    };

    class root_signature : public release_ptr<D3D12RootSignature> {
    public:
        root_signature(device&);
    };
    
    class cb_root_signature : public release_ptr<D3D12RootSignature> {
    public:
        cb_root_signature(device&);
        cb_root_signature(device&,int);
    };

    class cs_root_signature : public release_ptr<D3D12RootSignature> {
    public:
        cs_root_signature(device&);
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
        pipeline_state_object(device&, cb_root_signature&, vertex_shader&, pixel_shader&);
        pipeline_state_object(device&, cb_root_signature&, compute_shader&);
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
        constant_buffer(device&,resource_descriptor_heap&,std::pair<char*,size_t>);
        void update(std::pair<char*,size_t>);
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
        rtv_descriptor_heap rtv_heap_;
        resource_descriptor_heap srv_heap_;
    public:
        rtv_srv_texture2D(swap_chain&, unsigned num_buffers, tags::shader_visible);

        resource_descriptor_heap& srv_heap() { return srv_heap_; }
        rtv_descriptor_heap& rtv_heap() { return rtv_heap_; }
    };

    class uav_texture2D : public release_ptr<D3D12Resource> {
        //resource_descriptor_heap uav_heap_;
        //resource_descriptor_heap srv_heap_;        
        //rtv_descriptor_heap rtv_heap_;        
    public:
        uav_texture2D(swap_chain&, D3D12_CPU_DESCRIPTOR_HANDLE&);

        //resource_descriptor_heap& srv_heap() { return srv_heap_; }
        //resource_descriptor_heap& uav_heap() { return uav_heap_; }
        //rtv_descriptor_heap& rtv_heap() { return rtv_heap_; }
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
    //    size_t frame_index;
    //public:
    //    swap_chain(gtl::window&, command_queue&);                        
    //};    
    //
    //class rtv_descriptor_heap : public detail::simple_ptr_base<D3D12DescriptorHeap> {
    //    size_t increment;
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
    //class swapchain_rtv_heap {
    //    std::array<resource,frame_count()> frames;
    //public:
    //    swapchain_rtv_heap(swap_chain&, device&, rtv_descriptor_heap&);
    //};


}

}} // namespaces
#endif

