#include "../include/d3d_types.h"
#include "../include/d3d_funcs.h"

#include <gtl/include/gtl_window.h>
#include <gtl/include/release_ptr.h>
#include <gtl/include/win_tools.h>
#include <gtl/include/file_utilities.h>

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
        
    static void set_name(ID3D12Object* t, wchar_t const* name) 
    {   
        t->SetName(name);
    }


    debug_layer::debug_layer()
    {
        throw_on_fail(D3D12GetDebugInterface(__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);     	
    	get()->EnableDebugLayer();        
    }

    device::device() 
    {   
        throw_on_fail(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(type),
                                        reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);        
    }
    
    command_queue::command_queue(device& dev)
    {        
        D3D12_COMMAND_QUEUE_DESC cq_desc{};
        cq_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cq_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        throw_on_fail(dev->CreateCommandQueue(&cq_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);    
        set_name(get(),L"command_queue");               
    }

    swap_chain::swap_chain(gtl::window& win, command_queue& cqueue, unsigned const num_buffers_) 
        :   frame_resources(num_buffers_),
            rtv_heap{get_device(cqueue), num_buffers_, tags::not_shader_visible{}}     
    {
        RECT client_area{};
        if (!GetClientRect(get_hwnd(win), &client_area)) { throw std::runtime_error{__func__}; }        
        auto desc = create_swapchain_desc(tags::flipmodel_windowed{}, get_hwnd(win), num_buffers_, width(client_area), height(client_area));                
  
        release_ptr<IDXGISwapChain> tmp_ptr;  // We must first initialize an IDXGISwapChain* and then 
                                              // use QueryInterface() to "upcast" to our desired type,
                                              // in this case IDXGISwapChain3 (ie., DXGISwapChain)
        throw_on_fail(get_dxgi_factory()->CreateSwapChain(cqueue.get(), &desc, &tmp_ptr),__func__);             
        throw_on_fail(tmp_ptr->QueryInterface(__uuidof(type),reinterpret_cast<void**>(&expose_ptr())),__func__); 
        
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{rtv_heap->GetCPUDescriptorHandleForHeapStart()};        

        device dev{get_device(cqueue)};

        for (UINT idx = 0; idx < frame_resources.size(); ++idx) {
            throw_on_fail(this->get()->GetBuffer(idx,__uuidof(resource::type),
                              reinterpret_cast<void**>(&frame_resources[idx])),__func__);
            dev->CreateRenderTargetView(frame_resources[idx], nullptr, handle);
            handle.Offset(1, rtv_heap.increment_value());			
            set_name(frame_resources[idx].get(),L"swchn_rtv");                       
	    }          

        get()->SetMaximumFrameLatency(num_buffers_);                  
    }

    
    rtv_descriptor_heap::rtv_descriptor_heap(device& dev, unsigned num_descriptors, tags::not_shader_visible) 
    {        
        D3D12_DESCRIPTOR_HEAP_DESC dh_desc{};
		dh_desc.NumDescriptors = num_descriptors;
		dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;    // We only use these as targets       
        throw_on_fail(dev->CreateDescriptorHeap(&dh_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        increment_ = dev->GetDescriptorHandleIncrementSize(dh_desc.Type);        
        set_name(get(),L"rtv");              
    }    

    resource_descriptor_heap::resource_descriptor_heap(device& dev, unsigned num_descriptors, tags::shader_visible) 
    {
        D3D12_DESCRIPTOR_HEAP_DESC dh_desc{};
		dh_desc.NumDescriptors = num_descriptors;
		dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		       
        throw_on_fail(dev->CreateDescriptorHeap(&dh_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);

        increment_ = dev->GetDescriptorHandleIncrementSize(dh_desc.Type);
        set_name(get(),L"cbv");               
    }

    sampler_descriptor_heap::sampler_descriptor_heap(device& dev) 
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		
        
        throw_on_fail(dev->CreateDescriptorHeap(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);

        increment_ = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_name(get(),L"sdh");               
    }
 
 
    //swapchain_rtv_heap::swapchain_rtv_heap(swap_chain& swchain, device& dev, rtv_descriptor_heap& rtvheap) 
    //{
    //    DXGI_SWAP_CHAIN_DESC1 swchain_desc;
    //    throw_on_fail(swchain->GetDesc1(&swchain_desc),__func__);        
    //    frames.resize(swchain_desc.BufferCount);
    //    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvheap->GetCPUDescriptorHandleForHeapStart());
	//	
	//	for (UINT i = 0; i < frames.size(); ++i) {
    //        throw_on_fail(swchain->GetBuffer(i, __uuidof(resource::type), 
    //                      reinterpret_cast<void**>(&frames[i]))
    //                      ,__func__);
    //        dev->CreateRenderTargetView(frames[i], nullptr, handle);
    //        handle.Offset(1,rtvheap.increment_value());			
    //        set_name(frames[i].get(),L"rtv_swchn");               
	//    }                
    //}

    direct_command_allocator::direct_command_allocator(device& dev)
    {
        throw_on_fail(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);              
        set_name(get(),L"dca");               
    }

    direct_command_allocator::~direct_command_allocator()
    {
        get()->Reset();
    }

    root_signature::root_signature(device& dev)
    {
        CD3DX12_ROOT_SIGNATURE_DESC desc{};
		desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);        

        release_ptr<D3DBlob> signature;
        release_ptr<D3DBlob> error;
        
        HRESULT res1, res2;

        throw_on_fail(res1 = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)
                      ,__func__);       
        throw_on_fail(res2 = dev->CreateRootSignature(0, signature->GetBufferPointer(),                                                           signature->GetBufferSize(), 
                                        __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);	            
        set_name(get(),L"rootsig");               
    }

    cb_root_signature::cb_root_signature(device& dev)
    {
        std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
		std::vector<CD3DX12_ROOT_PARAMETER> rootParameters; 
		
        ranges.resize(3); 
        rootParameters.resize(3);

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		        
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);		        
        rootParameters[0].InitAsConstantBufferView(0);        
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[2].InitAsDescriptorTable(2, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
        
		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, 
                                rootSignatureFlags);

        release_ptr<D3DBlob> signature;
        release_ptr<D3DBlob> error;

		throw_on_fail(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)
                      ,__func__);
		throw_on_fail(dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), 
                                                __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);
        set_name(get(),L"cbsig");               
    }



    vertex_shader::vertex_shader(std::wstring path) 
    {                                
        throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr())
                      ,__func__);        
    }
    
    pixel_shader::pixel_shader(std::wstring path) 
    {                        
        throw_on_fail(D3DReadFileToBlob(path.c_str(),&expose_ptr())
                      ,__func__);        
    }
     
    srv::srv(device& dev, resource_descriptor_heap& cbvheap, command_queue& cqueue_, std::wstring filename)
    {
        //throw_on_fail(dev->CreateCommittedResource(
        //                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        //                    D3D12_HEAP_FLAG_NONE,
        //                    &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
	    //                    D3D12_RESOURCE_STATE_GENERIC_READ,
	    //                    nullptr,
        //                    __uuidof(resource::type),
        //                    reinterpret_cast<void**>(&buffer.expose_ptr()))            
        //             ,__func__);

        release_ptr<D3D12Resource> texture;        
        D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc{};        
        
        HRESULT result = DirectX::CreateDDSTextureFromFile(dev,filename.c_str(),&texture.expose_ptr(),&srvdesc);
        throw_on_fail(result,__func__);

        D3D12_RESOURCE_DESC desc = texture->GetDesc();

        HRESULT result2 = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                   nullptr,
                                                   __uuidof(type),
                                                   reinterpret_cast<void**>(&expose_ptr()));
        throw_on_fail(result2,__func__);
            
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

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(cbvheap->GetCPUDescriptorHandleForHeapStart());
         
        dev->CreateShaderResourceView(get(), &srvdesc, handle);               

        set_name(get(),L"srv");               
    }

    pipeline_state_object::pipeline_state_object(device& dev, cb_root_signature& rsig, 
                                                 vertex_shader& vs, pixel_shader& ps)
    {        	    
        D3D12_STREAM_OUTPUT_DESC sodesc{};
        
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		//desc.InputLayout = { nullptr, 0};
		desc.pRootSignature = rsig.get();
		desc.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
		desc.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
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
        throw_on_fail(result,__func__);
        set_name(get(),L"pso");               
    }

    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc, pipeline_state_object& pso)
    {
        throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                pso.get(),__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clwpso");               
	}

    graphics_command_list::graphics_command_list(device& dev, direct_command_allocator& alloc)
    {
        throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                nullptr,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clnopso");               
	}
        
    fence::fence(D3D12Device& dev)         
    {
        throw_on_fail(dev.CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__); 
        set_name(get(),L"fence");               
    }

    void fence::synchronized_set(uint64_t new_value, command_queue& cqueue)
    {
        fence tmp_fence_{ get_device(cqueue) };
        gtl::win::waitable_handle handle;        
        tmp_fence_->SetEventOnCompletion(tmp_fence_->GetCompletedValue()+1, handle);        
        cqueue->Signal(this->get(),new_value);
        cqueue->Signal(tmp_fence_.get(), tmp_fence_->GetCompletedValue()+1);        
        wait(handle);
        assert(this->get()->GetCompletedValue() == new_value);
    }
        
    constant_buffer::constant_buffer(device& dev, resource_descriptor_heap& cbvheap, std::pair<char*,size_t> cbuf)    
    {
        throw_on_fail(dev->CreateCommittedResource(
                            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                            D3D12_HEAP_FLAG_NONE,
                            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
	                        D3D12_RESOURCE_STATE_GENERIC_READ,
	                        nullptr,
                            __uuidof(resource::type),
                            reinterpret_cast<void**>(&buffer.expose_ptr()))            
                     ,__func__);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	    cbvDesc.BufferLocation = buffer.get()->GetGPUVirtualAddress();
	    cbvDesc.SizeInBytes = (cbuf.second + 255) & ~255;	// CB size is required to be 256-byte aligned.
	    dev->CreateConstantBufferView(&cbvDesc, cbvheap.get()->GetCPUDescriptorHandleForHeapStart());
    
		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
        throw_on_fail(buffer.get()->Map(0, nullptr, reinterpret_cast<void**>(&cbv_data_ptr))
                      ,__func__);
        set_name(buffer.get(),L"cbuf");                       
        this->update(cbuf);                
	}
    
    void constant_buffer::update(std::pair<char*,size_t> cbuf) 
    {
        std::memcpy(cbv_data_ptr, cbuf.first, cbuf.second);    	
    }

    sampler::sampler(device& dev, sampler_descriptor_heap& smpheap)
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

	    dev->CreateSampler(&desc, smpheap.get()->GetCPUDescriptorHandleForHeapStart());    	
        //set_name(get(),L"samplers");               
    }

    rtv_srv_texture2D::rtv_srv_texture2D(swap_chain& swchain, unsigned num_buffers, tags::shader_visible)
        :   rtv_heap_{ get_device(swchain), num_buffers, tags::not_shader_visible{}},
            srv_heap_{ get_device(swchain), 1, tags::shader_visible{}}
    {
        DXGI_SWAP_CHAIN_DESC swchaindesc_{};
        swchain->GetDesc(&swchaindesc_);

        device dev{get_device(swchain)};

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
