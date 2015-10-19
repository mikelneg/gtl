#include "../include/d3d_types.h"
#include "../include/d3d_funcs.h"

#include <gtl/include/gtl_window.h>
#include <gtl/include/release_ptr.h>
#include <gtl/include/win_tools.h>
#include <gtl/include/file_utilities.h>

#include <d3dcompiler.h>

#include <DDSTextureLoader.h>

#include <string>
#include <locale>
#include <codecvt>
#include <cstring>
#include <vector>
#include <atomic>

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
        auto cq_desc = command_queue_desc();        
        throw_on_fail(dev->CreateCommandQueue(&cq_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);    
        set_name(get(),L"command_queue");               
    }

    swap_chain::swap_chain(gtl::window& win, command_queue& cqueue) 
    {
        RECT client_area{};
        GetClientRect(get_hwnd(win), &client_area);
                                                                
        auto swch_desc = swchain_desc(get_hwnd(win),width(client_area),height(client_area));                

        release_ptr<IDXGISwapChain> tmp_ptr;  // we have to use a Base* type and then QueryInterface() after to upcast
        throw_on_fail(get_dxgi_factory()->CreateSwapChain(cqueue.get(), &swch_desc, &tmp_ptr)
                      ,__func__);             

        throw_on_fail(tmp_ptr->QueryInterface(__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__); 
                
        frame_index = get()->GetCurrentBackBufferIndex();            
    }

    void swap_chain::update_frame_index()
    {
        frame_index = get()->GetCurrentBackBufferIndex();    
    }

    rtv_descriptor_heap::rtv_descriptor_heap(device& dev) 
    {
        auto dh_desc = rtv_descriptor_heap_desc();   
        throw_on_fail(dev->CreateDescriptorHeap(&dh_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);

        increment = dev->GetDescriptorHandleIncrementSize(dh_desc.Type);
        set_name(get(),L"rtv");               
    }

    cbv_descriptor_heap::cbv_descriptor_heap(device& dev) 
    {
        auto dh_desc = cbv_descriptor_heap_desc();   
        throw_on_fail(dev->CreateDescriptorHeap(&dh_desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);

        increment = dev->GetDescriptorHandleIncrementSize(dh_desc.Type);
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

        increment = dev->GetDescriptorHandleIncrementSize(desc.Type);
        set_name(get(),L"sdh");               
    }
 
 
    rtv_frame_resources::rtv_frame_resources(swap_chain& swchain, device& dev, rtv_descriptor_heap& rtvheap) 
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvheap->GetCPUDescriptorHandleForHeapStart());
		
		for (UINT i = 0; i < frames.size(); ++i) {
            throw_on_fail(swchain->GetBuffer(i, __uuidof(resource::type), reinterpret_cast<void**>(&frames[i]))
                          ,__func__);
            dev->CreateRenderTargetView(frames[i], nullptr, handle);
            handle.Offset(1,rtvheap.size());			
            set_name(frames[i].get(),L"rtvframes");               
	    }                
    }

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
        CD3DX12_DESCRIPTOR_RANGE ranges[2];
		CD3DX12_ROOT_PARAMETER rootParameters[3]; 
		
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		        
        rootParameters[0].InitAsConstantBufferView(0);        
        rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
        
		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, 
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
     
    srv::srv(device& dev, cbv_descriptor_heap& cbvheap, command_queue& cqueue_, direct_command_allocator& calloc, fence& fence_, std::wstring filename)
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

        gtl::d3d::command_list clist{dev,calloc};

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

        fence_.wait_for_gpu(cqueue_);


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
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;        
		HRESULT result = dev->CreateGraphicsPipelineState(&desc,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()));
        throw_on_fail(result,__func__);
        set_name(get(),L"pso");               
    }

    command_list::command_list(device& dev, direct_command_allocator& alloc, pipeline_state_object& pso)
    {
        throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                pso.get(),__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clwpso");               
	}

    command_list::command_list(device& dev, direct_command_allocator& alloc)
    {
        throw_on_fail(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.get(), 
                                nullptr,__uuidof(type),reinterpret_cast<void**>(&expose_ptr()))
                     ,__func__);

        throw_on_fail(get()->Close(),__func__);
        set_name(get(),L"clnopso");               
	}

    
    fence::fence(device& dev)         
    {
        throw_on_fail(dev->CreateFence(frame_count(), D3D12_FENCE_FLAG_NONE, __uuidof(type), reinterpret_cast<void**>(&expose_ptr()))
                      ,__func__);

        fence_value.store(frame_count());

        event_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (event_handle == nullptr)
		{
			throw_on_fail(HRESULT_FROM_WIN32(GetLastError())
                          ,__func__);
		}        
        set_name(get(),L"fence");               
    }

    void fence::wait_for_frames(command_queue& cqueue_)
    {        
	    //auto const local_value = fence_value.load(std::memory_order_acquire);
        //throw_on_fail(cqueue_->Signal(get(),local_value)
        //              ,__func__);
        //
        //if (local_value == std::numeric_limits<decltype(local_value)>::max()) {
        //    fence_value.store(frame_count(),std::memory_order_release);
        //} else {
        //    fence_value.store(local_value+1,std::memory_order_release);
        //}
        //
	    //if (get()->GetCompletedValue() < local_value-1)
	    //{
	    //	throw_on_fail( get()->SetEventOnCompletion(local_value-1, event_handle), __func__);
	    //	WaitForSingleObject(event_handle, INFINITE);
	    //}                
        static uint64_t local_value{get()->GetCompletedValue()};

        cqueue_->Signal(get(),local_value);

        if (get()->GetCompletedValue() < local_value) {
            //cqueue_->Wait(get(), local_value);                        
            get()->SetEventOnCompletion(local_value, event_handle);
            WaitForSingleObject(event_handle, INFINITE);
        }

        if (local_value == std::numeric_limits<decltype(local_value)>::max() - frame_count()) {
            local_value = frame_count();
        } else {
            local_value++;
        }
    }

    void fence::wait_for_gpu(command_queue& cqueue_)
    {         
        uint64_t local_value{get()->GetCompletedValue()};
        
        cqueue_->Signal(get(),local_value+1);

        get()->SetEventOnCompletion(local_value+1, event_handle);
        WaitForSingleObject(event_handle, INFINITE);        
    }
        
    fence::~fence()
    {
        CloseHandle(event_handle);
    }

    constant_buffer::constant_buffer(device& dev, cbv_descriptor_heap& cbvheap, std::pair<char*,size_t> cbuf)    
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
