#include "gtl/d3d_types.h"
#include "gtl/d3d_funcs.h"

#include <iostream>
#include <cstddef>

#include <gtl/gtl_window.h>
#include <gtl/release_ptr.h>
#include <gtl/win_tools.h>
#include <gtl/file_utilities.h>

#include <d3dcompiler.h>

#include <DDSTextureLoader.h>

#include <chrono>
#include <string>
#include <locale>
#include <codecvt>
#include <cstring>
#include <vector>
#include <atomic>
#include <cassert>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl { 
namespace d3d { 
namespace _12_0 {

    using namespace gtl::win;    
        
    static 
    void set_name(ID3D12Object* t, wchar_t const* name) 
    {   
        t->SetName(name);
    }

    static
    void enable_debug()
    {
        release_ptr<D3D12Debug> debug_;
        win::throw_on_fail(D3D12GetDebugInterface(__uuidof(D3D12Debug),reinterpret_cast<void**>(&debug_.expose_ptr())),__func__);      
        debug_->EnableDebugLayer();
    }


    dxgi_factory::dxgi_factory()
    {          
        win::throw_on_fail(CreateDXGIFactory2(0,__uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                            ,__func__ );                
    }
    
    device::device(gtl::tags::debug) 
    {  
        enable_debug();    
        win::throw_on_fail(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(type),
                                        reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);        
    }
    
    device::device(gtl::tags::release) 
    {   
        win::throw_on_fail(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(type),
                                        reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);        
    }

    command_queue::command_queue(device& dev)
    {        
        D3D12_COMMAND_QUEUE_DESC cq_desc{};
        cq_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cq_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        win::throw_on_fail(dev->CreateCommandQueue(&cq_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);    
        set_name(get(),L"cqueue");               
    }

    rtv_descriptor_heap::rtv_descriptor_heap(device& dev, std::vector<resource>& resources_) 
        : size_{static_cast<unsigned>(resources_.size())}
    {        
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = size_;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;       
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);         
        set_name(get(),L"rtv_heap_");              
    }  

    rtv_descriptor_heap::rtv_descriptor_heap(device& dev, unsigned num_descriptors) 
        : size_{num_descriptors}
    {        
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = size_;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;       
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);         
        set_name(get(),L"rtv_heap_");              
    }  
    
    resource_descriptor_heap::resource_descriptor_heap(device& dev, unsigned num_descriptors, d3d::tags::shader_visible) 
        : size_{num_descriptors}
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;                
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_name(get(),L"res_heap");               
    }

    sampler_descriptor_heap::sampler_descriptor_heap(device& dev, unsigned num_descriptors) 
        : size_{num_descriptors}
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = num_descriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		        
        win::throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_name(get(),L"samp_heap");               
    }

    swap_chain::swap_chain(gtl::win::window& win, command_queue& cqueue_, unsigned num_buffers_) 
        :   frames_(num_buffers_),
            rtv_heap_{get_device_from(cqueue_), num_buffers_}     
    {        
        RECT client_area{};
        if (!GetClientRect(get_hwnd(win), &client_area)) { throw std::runtime_error{__func__}; }        
        auto desc = create_swapchain_desc(tags::flipmodel_windowed{}, get_hwnd(win), num_buffers_, width(client_area), height(client_area));                                  
        release_ptr<IDXGISwapChain> tmp_ptr;  // We first get a generic IDXGISwapChain* and then 
                                              //  use QueryInterface() to "upcast" to our desired type,
                                              //  in this case IDXGISwapChain3 (ie., DXGISwapChain)
        win::throw_on_fail(dxgi_factory{}->CreateSwapChain(cqueue_.get(), &desc, &tmp_ptr),__func__);             
        win::throw_on_fail(tmp_ptr->QueryInterface(__uuidof(type),reinterpret_cast<void**>(&expose_ptr())),__func__);                 

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};                
        auto dev_ = get_device_from(cqueue_);        
        for (unsigned i = 0; i < frames_.size(); ++i) {
            win::throw_on_fail(get()->GetBuffer(i, __uuidof(resource::type),reinterpret_cast<void**>(&frames_[i])),__func__);
            dev_->CreateRenderTargetView(frames_[i], nullptr, handle);
            handle.Offset(1, rtv_heap_.increment_value());			
            set_name(frames_[i].get(),L"swchn_rtv");                       
	    }          
        get()->SetMaximumFrameLatency(num_buffers_);                  
    }

    direct_command_allocator::direct_command_allocator(device& dev)
    {
        win::throw_on_fail(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);              
        set_name(get(),L"dc_alloc");               
    }

    compute_command_allocator::compute_command_allocator(device& dev)
    {
        win::throw_on_fail(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);              
        set_name(get(),L"cc_alloc");               
    }
    
    root_signature::root_signature(device& dev, blob signature_)
    {        
        release_ptr<D3DBlob> error_; // not currently using		
		win::throw_on_fail(dev->CreateRootSignature(0, signature_->GetBufferPointer(), signature_->GetBufferSize(),__uuidof(type), reinterpret_cast<void**>(&expose_ptr())),__func__);
        set_name(get(),L"root_sig");             
    }

    root_signature::root_signature(device& dev, vertex_shader& shader_)
    {        
        release_ptr<D3DBlob> error_; // not currently using		        
		win::throw_on_fail(dev->CreateRootSignature(0, shader_->GetBufferPointer(), shader_->GetBufferSize(),__uuidof(type), reinterpret_cast<void**>(&expose_ptr())),__func__);
        set_name(get(),L"root_sig");             
    }
    

  //  cb_root_signature::cb_root_signature(device& dev)
  //  {
  //      //
  //      //  RootSig(Sampler, Table{SRV,2}, Table{SRV,2});
  //      //           handler
  //           
  //      set_name(get(),L"cbsig");               
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
  //                                              __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
  //                    ,__func__);
  //      set_name(get(),L"cbsig"); 
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
  //                                              __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
  //                    ,__func__);
  //      set_name(get(),L"cbsig");               
  //  }



    vertex_shader::vertex_shader(std::wstring path) 
    {                                
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr())
                      ,__func__);        
    }
    
    pixel_shader::pixel_shader(std::wstring path) 
    {                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr())
                      ,__func__);        
    }
    
    compute_shader::compute_shader(std::wstring path) 
    {                        
        win::throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr())
                      ,__func__);        
    }
     
    srv::srv(device& dev, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles_, command_queue& cqueue_, std::wstring filename)
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

        release_ptr<D3D12Resource> texture;        
        D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc{};        
        
        HRESULT result = DirectX::CreateDDSTextureFromFile(dev,filename.c_str(),std::addressof(texture.expose_ptr()),&srvdesc);
        win::throw_on_fail(result,__func__);

        D3D12_RESOURCE_DESC desc = texture->GetDesc();

        HRESULT result2 = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),        
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_COMMON,
                                                   nullptr,
                                                   __uuidof(type),
                                                   reinterpret_cast<void**>(&expose_ptr()));
        win::throw_on_fail(result2,__func__);
            
        D3D12_RESOURCE_BARRIER barrierDesc{};

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

        ID3D12CommandList* ppCommandLists[] = { clist.get() };
        cqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);                
                
        wait_for_gpu(dev,cqueue_);
                 
        for (auto&& h : handles_) {
            dev->CreateShaderResourceView(get(), &srvdesc, h);               
        }

        set_name(get(),L"srv");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, root_signature& rsig, 
                                                 vertex_shader& vs, pixel_shader& ps)
    {        	    
        D3D12_STREAM_OUTPUT_DESC sodesc{};
        
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		//desc.InputLayout = { nullptr, 0};
		desc.pRootSignature = rsig.get();
		desc.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
		desc.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		
        //  
        desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);        
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
		HRESULT result = dev->CreateGraphicsPipelineState(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()));
        win::throw_on_fail(result,__func__);
        set_name(get(),L"pso-graphics");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, root_signature& rsig, compute_shader& cs)
    {        	                    
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		//desc.InputLayout = { nullptr, 0};
		desc.pRootSignature = rsig.get();
		desc.CS = { reinterpret_cast<UINT8*>(cs->GetBufferPointer()), cs->GetBufferSize() };				
        //D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG          
        HRESULT result = dev->CreateComputePipelineState(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()));
        win::throw_on_fail(result,__func__);
        set_name(get(),L"pso-compute");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, D3D12_GRAPHICS_PIPELINE_STATE_DESC const& desc_)
    {
        HRESULT result = dev->CreateGraphicsPipelineState(&desc_,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()));
        win::throw_on_fail(result,__func__);
        set_name(get(),L"pso-g-desc");               
    }


    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc, pipeline_state_object& pso)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                pso.get(),__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clpsod");               
	}

    graphics_command_list::graphics_command_list(device& dev, compute_command_allocator& alloc, pipeline_state_object& pso)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, alloc.get(), 
                                pso.get(),__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clpsoc");               
	}

    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc)
    {
        win::throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                nullptr,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        win::throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"cl");               
	}
        
    fence::fence(D3D12Device& dev)         
    {
        auto result = dev.CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()));
        win::throw_on_fail(result,__func__); 
        set_name(get(),L"fence");               
    }

    void fence::synchronized_set(uint64_t new_value, command_queue& cqueue)
    {
        fence tmp_fence_{ get_device_from(cqueue) };
        gtl::win::waitable_handle handle;        
        tmp_fence_->SetEventOnCompletion(tmp_fence_->GetCompletedValue()+1, handle);        
        cqueue->Signal(this->get(),new_value);
        cqueue->Signal(tmp_fence_.get(), tmp_fence_->GetCompletedValue()+1);        
        wait(handle);
        assert(this->get()->GetCompletedValue() == new_value);
    }
            

    constant_buffer::constant_buffer(device& dev, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor_handle, std::size_t cbuf_size)    
    {        
        win::throw_on_fail(dev->CreateCommittedResource(
                            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,
                            //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
                            &CD3DX12_RESOURCE_DESC::Buffer((cbuf_size + 255) & ~255),
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
                            //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	                        nullptr,
                            __uuidof(resource::type),
                            reinterpret_cast<void**>(&buffer.expose_ptr()))            
                     ,__func__);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	    cbvDesc.BufferLocation = buffer.get()->GetGPUVirtualAddress();
	    cbvDesc.SizeInBytes = (cbuf_size + 255) & ~255;	// CB size is required to be 256-byte aligned.
	    //dev->CreateConstantBufferView(&cbvDesc, resource_heap.get()->GetCPUDescriptorHandleForHeapStart());
        dev->CreateConstantBufferView(&cbvDesc, descriptor_handle);
    
		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        win::throw_on_fail(buffer.get()->Map(0, nullptr, reinterpret_cast<void**>(&cbv_data_ptr))
                      ,__func__);
        set_name(buffer.get(),L"cbuf");                               
	}

    constant_buffer::constant_buffer(device& dev, gtl::d3d::resource_descriptor_heap& rheap, std::size_t cbuf_size)    
        : constant_buffer(dev, rheap->GetCPUDescriptorHandleForHeapStart(),cbuf_size)
    { 
        // empty 
    }
    
    void constant_buffer::update(std::pair<char*,size_t> cbuf) 
    {
        std::memcpy(reinterpret_cast<void*>(cbv_data_ptr), reinterpret_cast<void*>(cbuf.first), cbuf.second);    	
    }

    void constant_buffer::update(char const* src,std::size_t size_)
    {
        std::memcpy(reinterpret_cast<void*>(cbv_data_ptr), reinterpret_cast<void const*>(src), size_);    	
    }

    sampler::sampler(device& dev, D3D12_CPU_DESCRIPTOR_HANDLE handle_)
    {
        D3D12_SAMPLER_DESC desc{};        	            
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
        //set_name(get(),L"samplers");               
    }

    sampler::sampler(device& dev, D3D12_SAMPLER_DESC const& desc, D3D12_CPU_DESCRIPTOR_HANDLE handle_)
    {
        dev->CreateSampler(&desc, handle_);    	
        //set_name(get(),L"samplers");               
    }

    rtv_srv_texture2D::rtv_srv_texture2D(swap_chain& swchain, DXGI_FORMAT format, unsigned num_buffers, d3d::tags::shader_visible)
        :   rtv_heap_{ get_device_from(swchain), num_buffers},
            srv_heap_{ get_device_from(swchain), 1, d3d::tags::shader_visible{}}
    {
        DXGI_SWAP_CHAIN_DESC swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        D3D12_TEXTURE_LAYOUT layout{};

        auto tdesc = CD3DX12_RESOURCE_DESC::Tex2D(format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  num_buffers, 1, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   nullptr,
                                                   __uuidof(type),
                                                   reinterpret_cast<void**>(&expose_ptr()));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
        CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};                    

        dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};

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

        set_name(get(), L"rtv_srv_tx2d");

    }

    rtv_srv_texture2D::rtv_srv_texture2D(swap_chain& swchain, unsigned num_buffers, d3d::tags::shader_visible)
        :   rtv_heap_{ get_device_from(swchain), num_buffers},
            srv_heap_{ get_device_from(swchain), 1, d3d::tags::shader_visible{}}
    {
        DXGI_SWAP_CHAIN_DESC swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        D3D12_TEXTURE_LAYOUT layout{};

        auto tdesc = CD3DX12_RESOURCE_DESC::Tex2D(swchaindesc_.BufferDesc.Format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  num_buffers, 1, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   nullptr,
                                                   __uuidof(type),
                                                   reinterpret_cast<void**>(&expose_ptr()));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
        CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};                    

        dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};

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

        set_name(get(), L"rtv_srv_tx2d");

    }


    uav_texture2D::uav_texture2D(swap_chain& swchain, D3D12_CPU_DESCRIPTOR_HANDLE& uav_handle)
     //: rtv_heap_{ get_device_from(swchain), 1, tags::not_shader_visible{}}
    {
        DXGI_SWAP_CHAIN_DESC swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device_from(swchain)};

        D3D12_TEXTURE_LAYOUT layout{};

        auto tdesc = CD3DX12_RESOURCE_DESC::Tex2D(swchaindesc_.BufferDesc.Format, 
                                                  swchaindesc_.BufferDesc.Width, 
                                                  swchaindesc_.BufferDesc.Height, 
                                                  1, 0, 1, 0, 
                                                  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                                  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,                                    
                                                   &tdesc,
                                                   D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                   nullptr,
                                                   __uuidof(type),
                                                   reinterpret_cast<void**>(&expose_ptr()));

        //CD3DX12_CPU_DESCRIPTOR_HANDLE uav_handle{uav_heap_->GetCPUDescriptorHandleForHeapStart()};        
        //CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle{srv_heap_->GetCPUDescriptorHandleForHeapStart()};        
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};        

        //dev->CreateShaderResourceView(get(), nullptr, srv_handle);
      
        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};


        uav_desc.Format = swchaindesc_.BufferDesc.Format;   // MAY work with this format.. may not..        
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        uav_desc.Texture2D.PlaneSlice = 0;
                    
        auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };
     
        //uav_desc.Texture2DArray.FirstArraySlice = i; // i: 0 -> numbuffers
        dev->CreateUnorderedAccessView(get(), nullptr, &uav_desc, uav_handle);        
        //uav_handle.Offset(1, rtv_heap_.increment_value());         

        //D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
        //
        //rtv_desc.Format = swchaindesc_.BufferDesc.Format;
        //rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        //rtv_desc.Texture2DArray.ArraySize = 1; // not num_buffers.. 
        //rtv_desc.Texture2DArray.MipSlice = 0;
        //rtv_desc.Texture2DArray.FirstArraySlice = 0;
        //
        //dev->CreateRenderTargetView(get(), &rtv_desc, rtv_handle);
        
        set_name(get(), L"rtv_srv_tx2d");
    }


//release_ptr<PixelShader>
//D3D11ResourceManager::
//createPixelShader(std::string filename_)
//{
//    std::vector<char> shader_ = game_utils::get_file(filename_);
//    PixelShader* tmp_{};    
//    device_.CreatePixelShader(shader_.data(), shader_.size(), nullptr, &tmp_);
//    if (tmp_ == nullptr) 
//        throw std::bad_exception{};
//    return release_ptr<PixelShader>{tmp_};
//}
//
//    }


}}} //namespaces
