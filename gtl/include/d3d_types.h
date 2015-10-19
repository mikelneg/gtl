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
#include <array>
#include <utility>
#include <atomic>
#include <string>

namespace gtl {    
namespace d3d {    

namespace _12_0 {    

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
    };    
                
    class command_queue : public release_ptr<D3D12CommandQueue> {        
    public:
        command_queue(device&);
    };

    class swap_chain : public release_ptr<DXGISwapChain> {        
        size_t frame_index;
    public:
        swap_chain(gtl::window&, command_queue&);
        void update_frame_index();
        size_t const& get_frame_index() const { return frame_index; }
    };    

    class rtv_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        UINT increment;
    public:
        rtv_descriptor_heap(device&);
        auto size() const noexcept { return increment; }
    };

    class cbv_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        UINT increment;
    public:
        cbv_descriptor_heap(device&);
        auto size() const noexcept { return increment; }
    };

    class sampler_descriptor_heap : public release_ptr<D3D12DescriptorHeap> {
        UINT increment;
    public:
        sampler_descriptor_heap(device&);
        auto size() const noexcept { return increment; }
    };

    class resource : public release_ptr<D3D12Resource> {
    public:        
        resource() = default;
    };
    
    class rtv_frame_resources {
        std::array<resource,frame_count()> frames;
    public:
        rtv_frame_resources(swap_chain&, device&, rtv_descriptor_heap&);
        decltype(frames) const& get_frames() const { return frames; }
    };

    class direct_command_allocator : public release_ptr<D3D12CommandAllocator> {
    public:
        direct_command_allocator(device&);
        ~direct_command_allocator();
    };

    class root_signature : public release_ptr<D3D12RootSignature> {
    public:
        root_signature(device&);
    };
    
    class cb_root_signature : public release_ptr<D3D12RootSignature> {
    public:
        cb_root_signature(device&);
    };

    class vertex_shader : public release_ptr<D3DBlob> {
    public:
        vertex_shader(std::wstring path);
    };

    class pixel_shader : public release_ptr<D3DBlob> {
    public:
        pixel_shader(std::wstring path);
    };

    class pipeline_state_object : public release_ptr<D3D12PipelineState> {
    public:
        pipeline_state_object(device&, cb_root_signature&, vertex_shader&, pixel_shader&);
    };

    class command_list : public release_ptr<D3D12GraphicsCommandList> {
    public:
        command_list(device&, direct_command_allocator&, pipeline_state_object&);
        command_list(device&, direct_command_allocator&);       
    };

    class fence : public release_ptr<D3D12Fence> {
        HANDLE event_handle{};
        std::atomic<uint64_t> fence_value{}; 
    public:
        fence(device&);
        void wait_for_gpu(command_queue&);
        void wait_for_frames(command_queue&);
        ~fence();
    };

    class constant_buffer {
        resource buffer;                
        unsigned char* cbv_data_ptr{};
    public:
        constant_buffer(device&,cbv_descriptor_heap&,std::pair<char*,size_t>);
        void update(std::pair<char*,size_t>);
        resource& resource() { return buffer; }
    };

    class srv : public release_ptr<D3D12Resource> {
    public:
        srv(device&,cbv_descriptor_heap&,command_queue&,direct_command_allocator&,fence&,std::wstring);
    };

    class sampler : public release_ptr<D3D12Resource> {
    public:
        sampler(device&,sampler_descriptor_heap&);
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
    //class rtv_frame_resources {
    //    std::array<resource,frame_count()> frames;
    //public:
    //    rtv_frame_resources(swap_chain&, device&, rtv_descriptor_heap&);
    //};


}

}} // namespaces
#endif

