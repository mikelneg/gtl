#include "gtl/d3d_types.h"
#include <gtl/d3d_helper_funcs.h>

#include <iostream>
#include <cstddef>

#include <gtl/gtl_window.h>
#include <gtl/intrusive_ptr.h>
#include <gtl/win_tools.h>
#include <gtl/file_utilities.h>

#include <d3dcompiler.h>

#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>

#include <DirectXHelpers.h>
#include <d3dx12.h>

#include <chrono>
#include <string>
#include <locale>
#include <codecvt>
#include <cstring>
#include <vector>
#include <atomic>
#include <ostream>
#include <cassert>
#include <algorithm>
#include <tuple>
#include <utility>

#include <gtl/d3d_ostream.h>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl { 
namespace d3d { 
namespace version_12_0 {    
    
    namespace { // internal helpers
        raw::SwapChainFullscreenDesc swchain_fullscreen_desc()
        {         
            raw::SwapChainFullscreenDesc desc{};
            desc.Windowed = true;
            desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;        
            desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            return desc;
        }
    
        raw::CommandQueueDesc command_queue_desc() 
        {
            raw::CommandQueueDesc desc{};
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            return desc;
        }
    
        raw::DescriptorHeapDesc rtv_descriptor_heap_desc() 
        {       
            raw::DescriptorHeapDesc desc{};
    		desc.NumDescriptors = frame_count();
    		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;		
            return desc;
        }
    
        raw::DescriptorHeapDesc resource_descriptor_heap_desc() 
        {       
            raw::DescriptorHeapDesc desc{};
    		desc.NumDescriptors = 1;
    		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		
            return desc;        
        }
    

        raw::SwapChainDesc create_swapchain_desc(tags::flipmodel_windowed, HWND hwnd, unsigned num_buffers, unsigned width, unsigned height) 
        {                       
            if (num_buffers < 2 || num_buffers > 16) { throw std::logic_error{__func__}; }        

            raw::SwapChainDesc desc{};        
            desc.BufferDesc.Width = width;
            desc.BufferDesc.Height = height;   // Rumor suggests this must be a multiple of 4..     
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;        
            //desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;        
            desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;                    
            desc.OutputWindow = hwnd;        
            desc.BufferCount = num_buffers;    // Required to be 2-16
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
            desc.SampleDesc.Count = 1;         // Required for flip-chain       
            desc.SampleDesc.Quality = 0;       // Required for flip-chain
            desc.Windowed = true;                
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // or _SEQUENTIAL -- 
            desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;       
            return desc;
        }
        
        template <typename T>
        void set_debug_name(T& t, wchar_t const* unicode_name) { t.SetName(unicode_name); }
                      
        void enable_debug() {
            release_ptr<raw::Debug> debug_;
            win::throw_on_fail( D3D12GetDebugInterface(__uuidof(raw::Debug), expose_as_void_pp(debug_)),
                                __func__);      
            debug_->EnableDebugLayer();
        }
    }

    dxgi_factory::dxgi_factory()
    {          
        win::throw_on_fail(CreateDXGIFactory2(0,__uuidof(type), expose_as_void_pp(*this)),__func__);                
    }
    
    device::device(gtl::tags::debug, std::ostream& str_) 
    {  
        enable_debug();
        auto adaptors = enumerate_adaptors();
        using gtl::d3d::operator<<;
        for (auto&& e : adaptors) {
            str_ << e;        
        }
        win::throw_on_fail(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(type),expose_as_void_pp(*this)),__func__);        
    }
    
    device::device(gtl::tags::release) 
    {   
        win::throw_on_fail(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(type),expose_as_void_pp(*this)),__func__);        
    }

    command_queue::command_queue(device& dev)
        : fence_{dev}
    {        
        D3D12_COMMAND_QUEUE_DESC cq_desc{};
        cq_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cq_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        win::throw_on_fail(dev->CreateCommandQueue(&cq_desc,__uuidof(type),expose_as_void_pp(*this)),__func__);    
        set_debug_name(*get(),L"cqueue");               
        synchronize();
    }

    void command_queue::synchronize() {
        fence_.synchronized_increment(*this);
    }

    //rtv_descriptor_heap::rtv_descriptor_heap(device& dev, std::vector<resource>& resources_) 
    //    : size_{static_cast<unsigned>(resources_.size())}
    //{        
    //    raw::DescriptorHeapDesc desc{};
	//	desc.NumDescriptors = size_;
	//	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;       
    //    win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),expose_as_void_pp(*this)),__func__);
    //    increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);         
    //    set_debug_name(*get(),L"rtv_desc_heap");              
    //}  

    rtv_descriptor_heap::rtv_descriptor_heap(device& dev, unsigned num_descriptors) 
        : size_{num_descriptors}
    {        
        raw::DescriptorHeapDesc desc{};
		desc.NumDescriptors = size_;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;       
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),expose_as_void_pp(*this)),__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);         
        set_debug_name(*get(),L"rtv_heap_");              
    }  
    
    resource_descriptor_heap::resource_descriptor_heap(device& dev, unsigned num_descriptors, d3d::tags::shader_visible) 
        : size_{num_descriptors}
    {
        raw::DescriptorHeapDesc desc{};
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;        
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),expose_as_void_pp(*this)),__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_debug_name(*get(),L"res_heap");               
    }

    resource_descriptor_heap::resource_descriptor_heap(device& dev, unsigned num_descriptors, d3d::tags::depth_stencil_view) 
        : size_{num_descriptors}
    {
        raw::DescriptorHeapDesc desc{};
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;        
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),expose_as_void_pp(*this)),__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_debug_name(*get(),L"res_heap_dsv");               
    }

    sampler_descriptor_heap::sampler_descriptor_heap(device& dev, unsigned num_descriptors) 
        : size_{num_descriptors}
    {
        raw::DescriptorHeapDesc desc{};
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		        
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),expose_as_void_pp(*this))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_debug_name(*get(),L"samp_heap");               
    }

    swap_chain::swap_chain(HWND hwnd, command_queue& cqueue_, unsigned num_buffers_) 
        :   frames_(num_buffers_),
            rtv_heap_{get_device_from(cqueue_), num_buffers_}     
    {        
        RECT client_area{};
        if (!GetClientRect(hwnd, &client_area)) { throw std::runtime_error{__func__}; }        
        //auto desc = create_swapchain_desc(tags::flipmodel_windowed{}, hwnd, num_buffers_, width(client_area), height(client_area));                                  
        auto desc = create_swapchain_desc(tags::flipmodel_windowed{}, hwnd, num_buffers_, 
                                            960, 540);                                  
        release_ptr<IDXGISwapChain> tmp_ptr;  // We first get a generic IDXGISwapChain* and then 
                                              // use QueryInterface() to get our desired type, raw::SwapChain                                              

        win::throw_on_fail(dxgi_factory{}->CreateSwapChain(cqueue_.get(), &desc, std::addressof(expose(tmp_ptr))),__func__);             
        win::throw_on_fail(tmp_ptr->QueryInterface(__uuidof(type),expose_as_void_pp(*this)),__func__);                 

        //raw::cx::CpuDescriptorHandle handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};                
        auto dev_ = get_device_from(cqueue_);  

        for (unsigned i = 0; i < frames_.size(); ++i) {
            win::throw_on_fail(get()->GetBuffer(i, __uuidof(resource::type),expose_as_void_pp(frames_[i])),__func__);
            //dev_->CreateRenderTargetView(frames_[i], nullptr, handle);
            dev_->CreateRenderTargetView(frames_[i], nullptr, rtv_heap_.get_handle(i));
            //handle.Offset(1, rtv_heap_.increment_value());			
            set_debug_name(*(frames_[i].get()),L"swchain");                       
	    }          
        get()->SetMaximumFrameLatency(num_buffers_);        
    }

    std::pair<int,int> swap_chain::resize(int new_width, int new_height) 
    {        
        // first release the current buffers
        auto sz = frame_count();
        frames_.clear();
        frames_.resize(sz);
        // then resize
        
        // HACK I should maintain old dimensions, verify new size, etc..
        
        DXGI_SWAP_CHAIN_DESC swdesc;
        get()->GetDesc(&swdesc);


        auto& hwnd = swdesc.OutputWindow;

        RECT client_rect_{};
        RECT window_rect_{};
        
        GetClientRect(hwnd, &client_rect_);
        GetWindowRect(hwnd, &window_rect_);                
                               
        std::cout << "old client width == " << client_rect_.right - client_rect_.left << "\n";
        std::cout << "old window width == " << window_rect_.right - window_rect_.left << "\n";


        DXGI_MODE_DESC new_desc = swdesc.BufferDesc;
        new_desc.Width = new_width;
        new_desc.Height = new_height;
                
        this->get()->ResizeTarget(&new_desc);

        GetClientRect(hwnd, &client_rect_);
        GetWindowRect(hwnd, &window_rect_);                
                               
        std::cout << "new client width == " << client_rect_.right - client_rect_.left << "\n";
        std::cout << "new window width == " << window_rect_.right - window_rect_.left << "\n";

        //GetClientRect(hwnd, &client_rect_);
        //GetWindowRect(hwnd, &window_rect_);                
        //                       
        //std::cout << "mid client width == " << client_rect_.right - client_rect_.left << "\n";
        //std::cout << "mid window width == " << window_rect_.right - window_rect_.left << "\n";
        //
        //
        //int dx = static_cast<int>((new_width - client_rect_.right) / 2);
        //int dy = static_cast<int>((new_height - client_rect_.bottom) / 2);
        //
        //InflateRect(&window_rect_, dx, dy);
        //        
        //new_desc.Width = window_rect_.right - window_rect_.left;
        //new_desc.Height = window_rect_.bottom - window_rect_.top;        
        //       
        //this->get()->ResizeTarget(&new_desc);
        //
        //GetClientRect(hwnd, &client_rect_);
        //GetWindowRect(hwnd, &window_rect_);        
        //
        //std::cout << "new client width == " << client_rect_.right - client_rect_.left << "\n";
        //std::cout << "new window width == " << window_rect_.right - window_rect_.left << "\n";

       // waitable = get()->GetFrameLatencyWaitableObject();
       // get()->Present1(0,DXGI_PRESENT_RESTART,std::addressof(p));
       // WaitForSingleObject(waitable,INFINITE);       
        
//        DXGI_PRESENT_PARAMETERS p{};
//        auto waitable = get()->GetFrameLatencyWaitableObject();
//        get()->Present1(0,DXGI_PRESENT_RESTART,std::addressof(p));
//        WaitForSingleObject(waitable,INFINITE);                     


        this->get()->ResizeBuffers(0, // maintains current buffer count
                             0,0, // uses client area dimensions
                             DXGI_FORMAT_UNKNOWN, // maintains current buffer format
                             DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT // HACK verify this (it raises issues surrounding fullscreen/non-fullscreen)
                             );
        
        
  //      waitable = get()->GetFrameLatencyWaitableObject();
  //      get()->Present1(0,DXGI_PRESENT_RESTART,std::addressof(p));
  //      WaitForSingleObject(waitable,INFINITE);       


        // then repopulate rtv_heap
        auto dev_ = get_device_from(*this);
        for (unsigned i = 0; i < frames_.size(); ++i) {
            win::throw_on_fail(get()->GetBuffer(i, __uuidof(resource::type),expose_as_void_pp(frames_[i])),__func__);
            //dev_->CreateRenderTargetView(frames_[i], nullptr, handle);
            dev_->CreateRenderTargetView(frames_[i], nullptr, rtv_heap_.get_handle(i));
            //handle.Offset(1, rtv_heap_.increment_value());			
            set_debug_name(*(frames_[i].get()),L"swchain");                       
	    }          


        return std::make_pair(client_rect_.right - client_rect_.left, client_rect_.bottom - client_rect_.top);
    }

    std::pair<unsigned,unsigned> swap_chain::dimensions() const 
    {
        raw::SwapChainDesc desc;        
        win::throw_on_fail(get()->GetDesc(&desc), __func__);    
        return std::make_pair(static_cast<unsigned>(desc.BufferDesc.Width),static_cast<unsigned>(desc.BufferDesc.Height));                            
    }

    raw::Viewport swap_chain::viewport() const 
    {
        raw::SwapChainDesc desc;        
        win::throw_on_fail(get()->GetDesc(&desc), __func__);    
        return raw::Viewport{0.0f,0.0f,static_cast<float>(desc.BufferDesc.Width),
                                       static_cast<float>(desc.BufferDesc.Height),
                                       0.0f, 1.0f};     // HACK using fixed values; should be determined..
    }
    
    unsigned swap_chain::frame_count() const 
    {
        return static_cast<unsigned>(frames_.size());        
    }

    direct_command_allocator::direct_command_allocator(device& dev)
    {
        win::throw_on_fail(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(type), expose_as_void_pp(*this))
                      ,__func__);              
        set_debug_name(*get(),L"direct_calloc");               
    }

    compute_command_allocator::compute_command_allocator(device& dev)
    {
        win::throw_on_fail(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, __uuidof(type), expose_as_void_pp(*this))
                      ,__func__);              
        set_debug_name(*get(),L"compute_calloc");               
    }            
    
    root_signature::root_signature(device& dev, blob signature_)
    {        
        release_ptr<raw::Blob> error_; // not currently using		
		win::throw_on_fail(dev->CreateRootSignature(0, signature_->GetBufferPointer(), signature_->GetBufferSize(),__uuidof(type), expose_as_void_pp(*this)),__func__);
        set_debug_name(*get(),L"root_sig");             
    }

    root_signature::root_signature(device& dev, vertex_shader& shader_)
    {        
        release_ptr<raw::Blob> error_; // not currently using		        
		win::throw_on_fail(dev->CreateRootSignature(0, shader_->GetBufferPointer(), shader_->GetBufferSize(),__uuidof(type), expose_as_void_pp(*this)),__func__);
        set_debug_name(*get(),L"root_sig");             
    }
    

  //  cb_root_signature::cb_root_signature(device& dev)
  //  {
  //      //
  //      //  RootSig(Sampler, Table{SRV,2}, Table{SRV,2});
  //      //           handler
  //           
  //      set_debug_name(*get(),L"cbsig");               
  //  }
  //  
  //  cb_root_signature::cb_root_signature(device& dev,int)
  //  {
  //      //
  //      //  RootSig(Sampler, Table{SRV,2}, Table{SRV,2});
  //      //           handler
  //      
  //            //
  //      //  RootSig(Sampler, Table{SRV,2}, Table{SRV,2});
  //      //           handler
  //      
  //      std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	//	std::vector<CD3DX12_ROOT_PARAMETER> params_; 
	//	
  //      ranges.resize(3); 
  //      params_.resize(2);                
  //              
  //      ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);        
  //      ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
  //      ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
  //      params_[0].InitAsDescriptorTable(2, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);                
  //      params_[1].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);                        
  //              
	//	// Allow input layout and deny uneccessary access to certain pipeline stages.
	//	D3D12_ROOT_SIGNATURE_FLAGS flags_ =
	////		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	//		//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
  //
	//	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//	rootSignatureDesc.Init(static_cast<UINT>(params_.size()), params_.data(), 0, nullptr, 
  //                              flags_);
  //
  //      release_ptr<D3DBlob> signature;
  //      release_ptr<D3DBlob> error;
  //
	//	win::throw_on_fail(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)
  //                    ,__func__);
	//	win::throw_on_fail(dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), 
  //                                              __uuidof(type), expose_as_void_pp(*this))
  //                    ,__func__);
  //      set_debug_name(*get(),L"cbsig"); 
  //  }
  //
  //  cs_root_signature::cs_root_signature(device& dev)
  //  {
  //      //
  //      //  RootSig(Sampler, Table{SRV,2}, Table{SRV,2});
  //      //           handler
  //      
  //      std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	//	std::vector<CD3DX12_ROOT_PARAMETER> params_; 
	//	
  //      ranges.resize(1); 
  //      params_.resize(1);                
  //
  //      ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);        
  //      params_[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);        
  //      //params_[0].InitAsUnorderedAccessView(0);
  //              
	//	// Allow input layout and deny uneccessary access to certain pipeline stages.
	//	D3D12_ROOT_SIGNATURE_FLAGS flags_ =
	////		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	//		//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
  //
	//	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//	rootSignatureDesc.Init(static_cast<UINT>(params_.size()), params_.data(), 0, nullptr, 
  //                              flags_);
  //
  //      release_ptr<D3DBlob> signature;
  //      release_ptr<D3DBlob> error;
  //
	//	win::throw_on_fail(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)
  //                    ,__func__);
	//	win::throw_on_fail(dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), 
  //                                              __uuidof(type), expose_as_void_pp(*this))
  //                    ,__func__);
  //      set_debug_name(*get(),L"cbsig");               
  //  }



    vertex_shader::vertex_shader(std::wstring path) 
    {                                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr()),__func__);        
    }
    
    geometry_shader::geometry_shader(std::wstring path) 
    {                                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr()),__func__);        
    }

    
    pixel_shader::pixel_shader(std::wstring path) 
    {                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr()),__func__);        
    }
    
    compute_shader::compute_shader(std::wstring path) 
    {                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr()),__func__);        
    }

    vertex_buffer::vertex_buffer(device& dev, command_queue& cqueue_, void* begin_, size_t size_)
    {        
        release_ptr<raw::Resource> upload_vbuffer_;                

        HRESULT result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &raw::cx::ResourceDesc::Buffer(size_),  // buffer alignment is 64k..
                                                   D3D12_RESOURCE_STATE_COPY_DEST,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));
        win::throw_on_fail(result2,__func__);
            
        result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &raw::cx::ResourceDesc::Buffer(size_),  // buffer alignment is 64k..
                                                   D3D12_RESOURCE_STATE_GENERIC_READ,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(upload_vbuffer_));        
        win::throw_on_fail(result2,__func__);
        
        void *buffer_ptr_{};
        win::throw_on_fail(
            upload_vbuffer_.get()->Map(0, nullptr, &buffer_ptr_), 
            __func__);
        //std::copy_n(begin_, size_, buffer_ptr_); 
        std::memcpy(buffer_ptr_, begin_, size_);    	
        upload_vbuffer_.get()->Unmap(0,nullptr);
        
        raw::ResourceBarrier barrierDesc{};

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = get();
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
              
        gtl::d3d::direct_command_allocator calloc{dev};
        gtl::d3d::graphics_command_list clist{dev,calloc};

        clist->Reset(calloc.get(), nullptr);                
        clist->CopyBufferRegion(get(),0,upload_vbuffer_.get(),0,size_);                  
        clist->ResourceBarrier(1, &barrierDesc);                 
        clist->DiscardResource(upload_vbuffer_.get(), nullptr);                
        clist->Close();

        raw::CommandList* ppCommandLists[] = { clist.get() };
        cqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);                
                
        cqueue_.synchronize();
        
        set_debug_name(*get(),L"vbuffer");               
    }

    index_buffer::index_buffer(device& dev, command_queue& cqueue_, void* begin_, size_t size_)
    {        
        resource upload_ibuffer_;                

        HRESULT result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &raw::cx::ResourceDesc::Buffer(size_),  // buffer alignment is 64k..
                                                   D3D12_RESOURCE_STATE_COPY_DEST,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));
        win::throw_on_fail(result2,__func__);
            
        result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &raw::cx::ResourceDesc::Buffer(size_),  // buffer alignment is 64k..
                                                   D3D12_RESOURCE_STATE_GENERIC_READ,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(upload_ibuffer_));        
        win::throw_on_fail(result2,__func__);
        
        void *buffer_ptr_{};
        win::throw_on_fail(
            upload_ibuffer_.get()->Map(0, nullptr, &buffer_ptr_), 
            __func__);
        //std::copy_n(begin_, size_, buffer_ptr_); 
        std::memcpy(buffer_ptr_, begin_, size_);    	
        upload_ibuffer_.get()->Unmap(0,nullptr);
        
        raw::ResourceBarrier barrierDesc{};

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = get();
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
              
        gtl::d3d::direct_command_allocator calloc{dev};
        gtl::d3d::graphics_command_list clist{dev,calloc};

        clist->Reset(calloc.get(), nullptr);                
        clist->CopyBufferRegion(get(),0,upload_ibuffer_.get(),0,size_);                  
        clist->ResourceBarrier(1, &barrierDesc);                 
        clist->DiscardResource(upload_ibuffer_.get(), nullptr);                
        clist->Close();

        raw::CommandList* ppCommandLists[] = { clist.get() };
        cqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);                
                
        cqueue_.synchronize();
        
        set_debug_name(*get(),L"ibuffer");               
    }


    srv::srv(device& dev, std::vector<raw::CpuDescriptorHandle> handles_, command_queue& cqueue_, std::wstring filename)
    {
        //win::throw_on_fail(dev->CreateCommittedResource(
        //                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        //                    D3D12_HEAP_FLAG_NONE,
        //                    &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
	    //                    D3D12_RESOURCE_STATE_GENERIC_READ,
	    //                    nullptr,
        //                    __uuidof(resource::type),
        //                    reinterpret_cast<void**>(&buffer.expose_ptr()))            
        //             ,__func__);

        // From docs on CreateDDSTextureFromFile...
        //Device->CreateCommittedResource(
        //&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        //D3D12_HEAP_FLAG_NONE,
        //&exampleResourceDesc,
        //D3D12_RESOURCE_STATE_COMMON,
        //nullptr,
        //IID_PPV_ARGS(exampleTexture.GetAddressOf()));

        release_ptr<raw::Resource> texture;        
                
        DirectX::ResourceUploadBatch res_batch{dev};

        res_batch.Begin();        

        bool is_cube{};

        HRESULT result = DirectX::CreateDDSTextureFromFile(dev,res_batch,filename.c_str(),&expose(texture),false,
                                                           0,nullptr,std::addressof(is_cube));

        //HRESULT result = DirectX::CreateDDSTextureFromFile(dev,filename.c_str(),&texture,&srvdesc); // pre-directxtk function                              

        res_batch.End(cqueue_);        

        win::throw_on_fail(result,__func__);
        
        raw::ResourceDesc desc = texture->GetDesc();

        HRESULT result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_COMMON,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));
        win::throw_on_fail(result2,__func__);               

        raw::ResourceBarrier barrierDesc{};

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = get();
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
      
        gtl::d3d::direct_command_allocator calloc{dev};
        gtl::d3d::graphics_command_list clist{dev,calloc};

        clist->Reset(calloc.get(), nullptr);        

        clist->ResourceBarrier(1, &barrierDesc);
        clist->CopyResource(get(), texture.get());
          
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        //The texture must be in D3D12_RESOURCE_STATE_GENERIC_READ before it can be sampled from
        clist->ResourceBarrier(1, &barrierDesc); 
        
        //Tell the GPU that it can free the heap 
        clist->DiscardResource(texture.get(), nullptr);        
        
        clist->Close();

        raw::CommandList* ppCommandLists[] = { clist.get() };
        cqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);                
                
        cqueue_.synchronize();
                 
        for (auto&& h : handles_) {
            //dev->CreateShaderResourceView(get(), &srvdesc, h);                           
            DirectX::CreateShaderResourceView(dev,*this,h,is_cube);
        }

        set_debug_name(*get(),L"srv");               
    }

    srv::srv(device& dev, std::vector<raw::CpuDescriptorHandle> handles_, command_queue& cqueue_, std::tuple<std::vector<uint32_t>,unsigned,unsigned> data)
    {
        release_ptr<raw::Resource> texture;        
                               
        auto align = [](unsigned x) { return (x+255) & ~255; };
        
        unsigned width = std::get<1>(data), height = std::get<2>(data);

        win::throw_on_fail(dev->CreateCommittedResource(
                            &raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,                            
                            &raw::cx::ResourceDesc::Buffer(align(align(width)*height*4)),
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
                            //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	                        nullptr,
                            __uuidof(resource::type),
                            expose_as_void_pp(texture))            
                     ,__func__);

        

		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        uint8_t* data_ptr{};
        win::throw_on_fail(texture.get()->Map(0, nullptr, reinterpret_cast<void**>(&data_ptr))
                      ,__func__);
        //std::copy_n(std::get<0>(data).begin(),std::get<0>(data).size(),data_ptr);
        

        //


D3D12_SUBRESOURCE_FOOTPRINT pitchedDesc{};
pitchedDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
pitchedDesc.Width = width;
pitchedDesc.Height = height;
pitchedDesc.Depth = 1;
pitchedDesc.RowPitch = ((width * sizeof(uint32_t))+255) & ~255;

//
// Note that the helper function UpdateSubresource in D3DX12.h, and ID3D12Device::GetCopyableFootprints 
// can help applications fill out D3D12_SUBRESOURCE_FOOTPRINT and D3D12_PLACED_SUBRESOURCE_FOOTPRINT structures.
//
// Refer to the D3D12 Code example for the previous section "Uploading Different Types of Resources"
// for the code for SuballocateFromBuffer.
//

uint8_t* pDataBegin = data_ptr;
uint8_t* pDataCur = data_ptr;
    
pDataCur = reinterpret_cast<UINT8*>((reinterpret_cast<size_t>(pDataCur)+255) & ~255);

D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D{};
placedTexture2D.Offset = pDataCur - pDataBegin;
placedTexture2D.Footprint = pitchedDesc;

for (unsigned y = 0; y < height; y++)
{
  UINT8 *pScan = pDataBegin + placedTexture2D.Offset + y * pitchedDesc.RowPitch;
  memcpy( pScan, std::get<0>(data).data() + (y * width), sizeof(DWORD) * width );
}

texture.get()->Unmap(0,nullptr);

cqueue_.synchronize();

        auto desc = raw::cx::ResourceDesc::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,width,height);

        HRESULT result2 = dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_COMMON,
                                                   nullptr,
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));
        win::throw_on_fail(result2,__func__);               
    
        raw::ResourceBarrier barrierDesc{};
    
        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = get();
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
      
        gtl::d3d::direct_command_allocator calloc{dev};
        gtl::d3d::graphics_command_list clist{dev,calloc};
    
        cqueue_.synchronize();

        clist->Reset(calloc.get(), nullptr);        
    
        clist->ResourceBarrier(1, &barrierDesc);
        //clist->CopyResource(get(), texture.get());
//        clist->CopyBufferRegion(get(),0,texture.get(),0,std::get<0>(data).size());

   
        clist->CopyTextureRegion( 
		    &CD3DX12_TEXTURE_COPY_LOCATION(get(),0), 
		    0, 0, 0, 
		    &CD3DX12_TEXTURE_COPY_LOCATION(texture.get(), placedTexture2D ), 
		    nullptr );



        //        
                  
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        //The texture must be in D3D12_RESOURCE_STATE_GENERIC_READ before it can be sampled from
        clist->ResourceBarrier(1, &barrierDesc); 
        
        //Tell the GPU that it can free the heap 
        clist->DiscardResource(texture.get(), nullptr);        
        
        clist->Close();
    
        raw::CommandList* ppCommandLists[] = { clist.get() };
        cqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);                
                
        cqueue_.synchronize();
                 
        for (auto&& h : handles_) {
            //dev->CreateShaderResourceView(get(), &srvdesc, h);                           
            DirectX::CreateShaderResourceView(dev,*this,h,false);
        }
    
        set_debug_name(*get(),L"srv");               
    }

    
    depth_stencil_buffer::depth_stencil_buffer(swap_chain& swchain)
        : buffer_view_{get_device(swchain),1,gtl::d3d::tags::depth_stencil_view{}}
    {
        auto dev = get_device(swchain);       
        auto dims = swchain.dimensions();
        //auto frame_count = swchain.frame_count(); 

        raw::cx::ResourceDesc desc{D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
                                   static_cast<UINT>(dims.first), 
                                   static_cast<UINT>(dims.second),
                                   1, 
                                   1, // mip levels
                                   DXGI_FORMAT_D32_FLOAT, 1, 0, 
                                   D3D12_TEXTURE_LAYOUT_UNKNOWN,
                                   D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE};

        D3D12_CLEAR_VALUE clear_value;
        clear_value.Format = DXGI_FORMAT_D32_FLOAT;
        clear_value.DepthStencil.Depth = 1.0f;
        clear_value.DepthStencil.Stencil = 0;
        
        raw::DsvDesc dsv_desc{};        
        dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;   
        dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

        win::throw_on_fail(dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                       D3D12_HEAP_FLAG_NONE, 
                                                       &desc,
                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE, 
                                                       &clear_value,
                                                       __uuidof(resource::type),
                                                       expose_as_void_pp(*this)), __func__);
                                    
        dev->CreateDepthStencilView(get(),&dsv_desc,buffer_view_->GetCPUDescriptorHandleForHeapStart());            
        set_debug_name(*get(),L"depth_stencil");               
    }  

    void depth_stencil_buffer::resize(int w, int h) {
        
        auto desc = this->get()->GetDesc();
        auto dev = get_device_from(this->get());               
        
        desc.Height = h;
        desc.Width = w;

        //raw::cx::ResourceDesc desc{D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
        //                           static_cast<UINT>(w), 
        //                           static_cast<UINT>(h),
        //                           1, 
        //                           1, // mip levels
        //                           DXGI_FORMAT_D32_FLOAT, 1, 0, 
        //                           D3D12_TEXTURE_LAYOUT_UNKNOWN,
        //                           D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE};

        D3D12_CLEAR_VALUE clear_value;
        clear_value.Format = desc.Format;
        clear_value.DepthStencil.Depth = 1.0f;
        clear_value.DepthStencil.Stencil = 0;
        
        raw::DsvDesc dsv_desc{};        
        dsv_desc.Format = desc.Format;
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;   
        dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

        win::throw_on_fail(dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                       D3D12_HEAP_FLAG_NONE, 
                                                       &desc,
                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE, 
                                                       &clear_value,
                                                       __uuidof(resource::type),
                                                       expose_as_void_pp(*this)), __func__);
                                    
        dev->CreateDepthStencilView(get(),&dsv_desc,buffer_view_->GetCPUDescriptorHandleForHeapStart());            
        set_debug_name(*get(),L"depth_stencil");               

    }

    pipeline_state_object::pipeline_state_object(device& dev, root_signature& rsig, 
                                                 vertex_shader& vs, pixel_shader& ps)
    {        	    
        raw::StreamOutputDesc sodesc{};
        
        raw::GraphicsPipelineStateDesc desc = {};
		//desc.InputLayout = { nullptr, 0};
		desc.pRootSignature = rsig.get();
		desc.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
		desc.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		desc.RasterizerState = raw::cx::RasterizerDesc(D3D12_DEFAULT);
		
        //  
        desc.BlendState = raw::cx::BlendDesc(D3D12_DEFAULT);        
        desc.BlendState.RenderTarget[0].BlendEnable = true;        
        desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; 
        desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;        
        //desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_BLEND_FACTOR;
        desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
        //desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_BLEND_FACTOR;        
       
		//
        
        desc.DepthStencilState.DepthEnable = FALSE;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.SampleMask = UINT_MAX;        
        //desc.StreamOutput = sodesc;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 2;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;              
		HRESULT result = dev->CreateGraphicsPipelineState(&desc,__uuidof(type),expose_as_void_pp(*this));
        win::throw_on_fail(result,__func__);
        set_debug_name(*get(),L"pso-graphics");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, root_signature& rsig, compute_shader& cs)
    {        	                    
        raw::ComputePipelineStateDesc desc{};
		//desc.InputLayout = { nullptr, 0};
		desc.pRootSignature = rsig.get();
		desc.CS = { reinterpret_cast<UINT8*>(cs->GetBufferPointer()), cs->GetBufferSize() };				
        //D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG          
        HRESULT result = dev->CreateComputePipelineState(&desc,__uuidof(type),expose_as_void_pp(*this));
        win::throw_on_fail(result,__func__);
        set_debug_name(*get(),L"pso-compute");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, raw::GraphicsPipelineStateDesc const& desc_)
    {
        HRESULT result = dev->CreateGraphicsPipelineState(&desc_,__uuidof(type),expose_as_void_pp(*this));
        win::throw_on_fail(result,__func__);
        set_debug_name(*get(),L"pso-g-desc");               
    }


    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc, pipeline_state_object& pso)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                pso.get(),__uuidof(type),expose_as_void_pp(*this))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_debug_name(*get(),L"clpsod");               
	}

    graphics_command_list::graphics_command_list(device& dev, compute_command_allocator& alloc, pipeline_state_object& pso)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, alloc.get(), 
                                pso.get(),__uuidof(type),expose_as_void_pp(*this))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_debug_name(*get(),L"clpsoc");               
	}

    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                nullptr,__uuidof(type),expose_as_void_pp(*this))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_debug_name(*get(),L"cl");               
	}
        
    fence::fence(device& dev)
    {
        auto result = dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(type), expose_as_void_pp(*this));
        win::throw_on_fail(result,__func__); 
        set_debug_name(*get(),L"fence");               
    }

    void fence::synchronized_increment(command_queue& cqueue)
    {
        gtl::win::waitable_handle handle;        
        auto const new_value = this->get()->GetCompletedValue()+1;
        this->get()->SetEventOnCompletion(new_value, handle);        
        cqueue->Signal(this->get(),new_value);        
        wait(handle);
        assert(this->get()->GetCompletedValue() == new_value);
    }
    
    void fence::synchronized_set(uint64_t new_value, command_queue& cqueue)
    {
        fence tmp_fence_{get_device_from(cqueue)};
        //gtl::win::waitable_handle handle;        
        //tmp_fence_->SetEventOnCompletion(tmp_fence_->GetCompletedValue()+1, handle);        
        cqueue->Signal(this->get(),new_value);
        tmp_fence_.synchronized_increment(cqueue); 
        //cqueue->Signal(tmp_fence_.get(), tmp_fence_->GetCompletedValue()+1);        
        //wait(handle);
        assert(this->get()->GetCompletedValue() == new_value);
    }
            
    constant_buffer::constant_buffer(device& dev, std::size_t cbuf_size)    
    {        
        win::throw_on_fail(dev->CreateCommittedResource(
                            &raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,
                            //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,                  
                            &raw::cx::ResourceDesc::Buffer((cbuf_size + 255) & ~255), // constant alignment is 256
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
                            //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	                        nullptr,
                            __uuidof(resource::type),
                            expose_as_void_pp(buffer))            
                     ,__func__);

		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        win::throw_on_fail(buffer.get()->Map(0, nullptr, reinterpret_cast<void**>(&cbv_data_ptr))
                      ,__func__);
        set_debug_name(*(buffer.get()),L"cbuf_no_hndl");                               
	}


    constant_buffer::constant_buffer(device& dev, raw::CpuDescriptorHandle descriptor_handle, std::size_t cbuf_size)    
    {        
        win::throw_on_fail(dev->CreateCommittedResource(
                            &raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,
                            //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,                  
                            &raw::cx::ResourceDesc::Buffer((cbuf_size + 255) & ~255), // constant alignment is 256
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
                            //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	                        nullptr,
                            __uuidof(resource::type),
                            expose_as_void_pp(buffer))            
                     ,__func__);

        raw::ConstantBufferViewDesc cbvDesc{};
	    cbvDesc.BufferLocation = buffer.get()->GetGPUVirtualAddress();        
	    cbvDesc.SizeInBytes = (cbuf_size + 255) & ~255;	// CB size is required to be 256-byte aligned.
	    //dev->CreateConstantBufferView(&cbvDesc, resource_heap.get()->GetCPUDescriptorHandleForHeapStart());
        dev->CreateConstantBufferView(&cbvDesc, descriptor_handle);
    
		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        win::throw_on_fail(buffer.get()->Map(0, nullptr, reinterpret_cast<void**>(&cbv_data_ptr))
                      ,__func__);
        set_debug_name(*(buffer.get()),L"cbuf");                               
	}

    constant_buffer::constant_buffer(device& dev, raw::CpuDescriptorHandle descriptor_handle, std::size_t size, d3d::tags::shader_view)    
    {        
        win::throw_on_fail(dev->CreateCommittedResource(
                            &raw::cx::HeapProperties(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,
                            //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,                  
                            &raw::cx::ResourceDesc::Buffer((size + 255) & ~255), // constant alignment is 256
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
                            //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	                        nullptr,
                            __uuidof(resource::type),
                            expose_as_void_pp(buffer))            
                     ,__func__);

        //raw::ConstantBufferViewDesc cbvDesc{};
        raw::SrvDesc srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.NumElements = static_cast<UINT>(size) / 4;
        srvDesc.Buffer.StructureByteStride = 0;

    //typedef struct D3D12_SHADER_RESOURCE_VIEW_DESC
    //{
    //DXGI_FORMAT Format;
    //D3D12_SRV_DIMENSION ViewDimension;
    //UINT Shader4ComponentMapping;
    //union 
    //    {
    //    D3D12_BUFFER_SRV Buffer;
    //    D3D12_TEX1D_SRV Texture1D;
    //    D3D12_TEX1D_ARRAY_SRV Texture1DArray;
    //    D3D12_TEX2D_SRV Texture2D;
    //    D3D12_TEX2D_ARRAY_SRV Texture2DArray;
    //    D3D12_TEX2DMS_SRV Texture2DMS;
    //    D3D12_TEX2DMS_ARRAY_SRV Texture2DMSArray;
    //    D3D12_TEX3D_SRV Texture3D;
    //    D3D12_TEXCUBE_SRV TextureCube;
    //    D3D12_TEXCUBE_ARRAY_SRV TextureCubeArray;
    //    } 	;
    //} 	D3D12_SHADER_RESOURCE_VIEW_DESC;

	    //srvDesc.BufferLocation = buffer.get()->GetGPUVirtualAddress();        
	    //srvDesc.SizeInBytes = (cbuf_size + 255) & ~255;	// CB size is required to be 256-byte aligned.
	    //dev->CreateConstantBufferView(&cbvDesc, resource_heap.get()->GetCPUDescriptorHandleForHeapStart());
        //dev->CreateConstantBufferView(&cbvDesc, descriptor_handle);
        dev->CreateShaderResourceView(buffer.get(), &srvDesc, descriptor_handle);
    
		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        win::throw_on_fail(buffer.get()->Map(0, nullptr, reinterpret_cast<void**>(&cbv_data_ptr))
                      ,__func__);
        set_debug_name(*(buffer.get()),L"cbuf");                               
	}


    constant_buffer::constant_buffer(device& dev, gtl::d3d::resource_descriptor_heap& rheap, std::size_t cbuf_size)    
        : constant_buffer(dev,rheap->GetCPUDescriptorHandleForHeapStart(),cbuf_size)
    { 
        // empty 
    }
    
    void constant_buffer::update(std::pair<char*,size_t> cbuf) 
    {
        std::memcpy(cbv_data_ptr, cbuf.first, cbuf.second);    	
    }

    void constant_buffer::update(char const* src, std::size_t size_)
    {
        std::memcpy(cbv_data_ptr, src, size_);   	
    }
    
    void constant_buffer::update(char const* src, std::size_t count, std::size_t offset) 
    {
        std::memcpy(cbv_data_ptr, src + offset, count);
    }


    sampler::sampler(device& dev, raw::CpuDescriptorHandle handle_)
    {
        raw::SamplerDesc desc{};        	            
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D12_FLOAT32_MAX;
        desc.MipLODBias = 0.0f;
        desc.MaxAnisotropy = 1;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	    dev->CreateSampler(&desc, handle_);    	
        //set_debug_name(*get(),L"samplers");               
    }

    sampler::sampler(device& dev, raw::SamplerDesc const& desc, raw::CpuDescriptorHandle handle_)
    {
        dev->CreateSampler(&desc, handle_);    	
        //set_debug_name(*get(),L"samplers");               
    }


    rtv_srv_texture2D::rtv_srv_texture2D(swap_chain& swchain, raw::Format format, unsigned num_buffers, d3d::tags::shader_visible)
        :   rtv_heap_{ get_device_from(swchain), num_buffers},
            srv_heap_{ get_device_from(swchain), 1, d3d::tags::shader_visible{}}
    {
        raw::SwapChainDesc swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        //D3D12_TEXTURE_LAYOUT layout{};

        auto tdesc = raw::cx::ResourceDesc::Tex2D(format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  num_buffers, 1, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clear_value_{};
        clear_value_.Format = format;        

        dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   std::addressof(clear_value_),
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));

        raw::cx::CpuDescriptorHandle rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
        raw::cx::CpuDescriptorHandle srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};                    

        dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        raw::RenderTargetViewDesc rtv_desc{};

        rtv_desc.Format = format;
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtv_desc.Texture2DArray.ArraySize = 1; // not num_buffers.. 
        rtv_desc.Texture2DArray.MipSlice = 0;
        //rtv_desc.Texture2DArray.FirstArraySlice = 0;        

        auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };

        for (UINT i = 0; i < num_buffers; ++i) {            
            rtv_desc.Texture2DArray.FirstArraySlice = i;
            dev->CreateRenderTargetView(get(), &rtv_desc, rtv_handle);
            rtv_handle.Offset(1, rtv_heap_.increment_value());
            
        }

        set_debug_name(*get(), L"rtv_srv_tx2d");

    }

    void rtv_srv_texture2D::resize(int w, int h) {
        auto desc = this->get()->GetDesc();
        auto dev = get_device_from(this->get());
 
        desc.Height = h;
        desc.Width = w;        

        auto const& num_buffers = desc.DepthOrArraySize;

        D3D12_CLEAR_VALUE clear_value_{};
        clear_value_.Format = desc.Format;

        dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   std::addressof(clear_value_),
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));

        raw::cx::CpuDescriptorHandle rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
        raw::cx::CpuDescriptorHandle srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};                    

        dev->CreateShaderResourceView(get(), nullptr, srv_handle);     

        raw::RenderTargetViewDesc rtv_desc{};

        rtv_desc.Format = desc.Format;
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtv_desc.Texture2DArray.ArraySize = 1; // not num_buffers.. 
        rtv_desc.Texture2DArray.MipSlice = 0;
        //rtv_desc.Texture2DArray.FirstArraySlice = 0;        

        auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };

        for (UINT i = 0; i < num_buffers; ++i) {            
            rtv_desc.Texture2DArray.FirstArraySlice = i;
            dev->CreateRenderTargetView(get(), &rtv_desc, rtv_handle);
            rtv_handle.Offset(1, rtv_heap_.increment_value());
            
        }        

        set_debug_name(*get(), L"rtv_srv_tx2d");

    }

    rtv_srv_texture2D::rtv_srv_texture2D(swap_chain& swchain, unsigned num_buffers, d3d::tags::shader_visible)
        :   rtv_heap_{ get_device_from(swchain), num_buffers},
            srv_heap_{ get_device_from(swchain), 1, d3d::tags::shader_visible{}}
    {
        raw::SwapChainDesc swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        raw::TextureLayout layout{};

        auto tdesc = raw::cx::ResourceDesc::Tex2D(swchaindesc_.BufferDesc.Format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  num_buffers, 1, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clear_value_{};
        clear_value_.Format = swchaindesc_.BufferDesc.Format;        

        dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   std::addressof(clear_value_),
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));

        raw::cx::CpuDescriptorHandle rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
        raw::cx::CpuDescriptorHandle srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};                    

        dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        raw::RenderTargetViewDesc rtv_desc{};

        rtv_desc.Format = swchaindesc_.BufferDesc.Format;
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtv_desc.Texture2DArray.ArraySize = 1; // not num_buffers.. 
        rtv_desc.Texture2DArray.MipSlice = 0;
        //rtv_desc.Texture2DArray.FirstArraySlice = 0;        

        auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };

        for (UINT i = 0; i < num_buffers; ++i) {            
            rtv_desc.Texture2DArray.FirstArraySlice = i;
            dev->CreateRenderTargetView(get(), &rtv_desc, rtv_handle);
            rtv_handle.Offset(1, rtv_heap_.increment_value());
            
        }

        set_debug_name(*get(), L"rtv_srv_tx2d");

    }


    uav_texture2D::uav_texture2D(swap_chain& swchain, raw::CpuDescriptorHandle& uav_handle)
     //: rtv_heap_{ get_device_from(swchain), 1, tags::not_shader_visible{}}
    {
        raw::SwapChainDesc swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        raw::TextureLayout layout{};

        auto tdesc = raw::cx::ResourceDesc::Tex2D(swchaindesc_.BufferDesc.Format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  1, 0, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clear_value_{};
        clear_value_.Format = swchaindesc_.BufferDesc.Format;                

        dev->CreateCommittedResource(&raw::cx::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                   std::addressof(clear_value_),
                                                   __uuidof(type),
                                                   expose_as_void_pp(*this));

        //CD3DX12_CPU_DESCRIPTOR_HANDLE uav_handle{uav_heap_->GetCPUDescriptorHandleForHeapStart()};        
        //CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};        
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};        

        //dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        raw::UnorderedAccessViewDesc uav_desc{};


        uav_desc.Format = swchaindesc_.BufferDesc.Format;   // MAY work with this format.. may not..        
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        uav_desc.Texture2D.PlaneSlice = 0;
                    
        auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };
             
        dev->CreateUnorderedAccessView(get(), nullptr, &uav_desc, uav_handle);                
        set_debug_name(*get(), L"rtv_srv_tx2d");
    }

}}} //namespaces
