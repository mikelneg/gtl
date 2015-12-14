#include "../include/d3d_funcs.h"
#include "../include/d3d_types.h"

#include <gtl/include/release_ptr.h>
#include <gtl/include/win_tools.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <windows.h>
#include <winerror.h>
#include <memory>
#include <stdexcept>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl { 
namespace d3d { 
namespace _12_0 {

    // static helper functions
    static 
    void disable_alt_enter(HWND hwnd)
    {
        auto factory = get_dxgi_factory();
        win::throw_on_fail( factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)
                            ,__func__ );
    }

    //////////////////////////////////////////////////////// 

    release_ptr<DXGIFactory> get_dxgi_factory() 
    {  
        release_ptr<DXGIFactory> ptr;
        win::throw_on_fail(CreateDXGIFactory2(0,__uuidof(DXGIFactory), reinterpret_cast<void**>(&ptr))
                            ,__func__ );        
        return ptr;
    }

    release_ptr<DXGIAdapter> get_hw_adapter()
    {	            
        // adapted from: https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/D3D12HelloWorld/src/HelloWindow/DXSample.cpp        
        auto factory = get_dxgi_factory();
        release_ptr<DXGIAdapter> ptr;                        
	    for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &ptr); ++i)
	    {
		    DXGI_ADAPTER_DESC1 desc;
		    ptr->GetDesc1(&desc);
		    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { 
		    	ptr.reset();
                continue;
		    }
		    if (win::succeeded(D3D12CreateDevice(ptr.get(), D3D_FEATURE_LEVEL_12_0, __uuidof(D3D12Device), nullptr))) {
		    	return ptr;
		    }
	    }	    

        win::throw_on_fail(E_FAIL, __func__); // if we've made it here, we have failed..
        return ptr; // silence compiler warning
    }

    DXGI_SWAP_CHAIN_DESC create_swapchain_desc(tags::flipmodel_windowed, HWND hwnd, unsigned num_buffers, unsigned width, unsigned height) 
    {           
        // some preconditions.. 
        if (num_buffers < 2 || num_buffers > 16) { throw std::logic_error{__func__}; }
        if ((height % 4) != 0) { throw std::logic_error{__func__}; }

        DXGI_SWAP_CHAIN_DESC desc{};        
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;   // Rumor suggests this must be a multiple of 4..     
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;        
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;        
        //desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;        
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

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swchain_fullscreen_desc()
    {         
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC desc{};
        desc.Windowed = true;
        desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;        
        desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        return desc;
    }

    D3D12_COMMAND_QUEUE_DESC command_queue_desc() 
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        return desc;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtv_descriptor_heap_desc() 
    {       
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = frame_count();
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;		
        return desc;
    }

    D3D12_DESCRIPTOR_HEAP_DESC resource_descriptor_heap_desc() 
    {       
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;		
        return desc;        
    }

    void report_live_objects(device& dev) 
    {
        gtl::release_ptr<gtl::d3d::D3D12DebugDevice> debug_device;
        dev->QueryInterface(__uuidof(gtl::d3d::D3D12DebugDevice),reinterpret_cast<void**>(&debug_device.expose_ptr()));            
        debug_device->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);            
    }

    void wait_for_gpu(device& dev, command_queue& cqueue_)
    {
        gtl::d3d::fence fence_{dev};
        gtl::win::waitable_handle handle_;
                
        fence_->SetEventOnCompletion(fence_->GetCompletedValue() + 1, handle_);
        cqueue_->Signal(fence_.get(),fence_->GetCompletedValue() + 1);
        WaitForSingleObject(handle_,INFINITE);        
    }
    
    release_ptr<D3DBlob> dummy_rootsig_1() 
    {        
        std::vector<CD3DX12_DESCRIPTOR_RANGE> table1_, table2_;
		std::vector<CD3DX12_ROOT_PARAMETER> params_; 
		
        table1_.resize(1);     
        table2_.resize(3);
        
        table1_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);   // 1 descriptor, register 0     
        
        table2_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		  // 1 descriptor, register 0
        table2_[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);    // 1 desc, reg 0, space 1
        table2_[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);       // 1 desc, reg 0
        
        params_.resize(3);
        params_[0].InitAsConstantBufferView(0); 
        params_[1].InitAsDescriptorTable(win::array_size(table1_), table1_.data(), D3D12_SHADER_VISIBILITY_PIXEL);
        params_[2].InitAsDescriptorTable(win::array_size(table2_), table2_.data(), D3D12_SHADER_VISIBILITY_ALL);
                
		D3D12_ROOT_SIGNATURE_FLAGS flags_ =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;			

		CD3DX12_ROOT_SIGNATURE_DESC desc;
		desc.Init(win::array_size(params_), params_.data(), 0, nullptr, flags_);    

        release_ptr<D3DBlob> signature_;
        release_ptr<D3DBlob> error_; // not currently using
        win::throw_on_fail(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, 
                                                  &(signature_.expose_ptr()), &(error_.expose_ptr()))
                                                  ,__func__);
        return signature_;        
    }

    release_ptr<D3DBlob> dummy_rootsig_2() 
    {
        std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
		std::vector<CD3DX12_ROOT_PARAMETER> params_; 
		
        ranges.resize(3); 
        params_.resize(2);                
                
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);        
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        params_[0].InitAsDescriptorTable(2, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);                
        params_[1].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);                        
                
		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS flags_ =
	//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(static_cast<UINT>(params_.size()), params_.data(), 0, nullptr, 
                                flags_);

        release_ptr<D3DBlob> signature;
        release_ptr<D3DBlob> error;

		win::throw_on_fail(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)
                      ,__func__);
		
        return signature;
    }


     /*               
            

        
    swapchain_ = CreateSwapChainForHwnd(hwnd, device.get_device_from(), desc, GetDefault<SWAP_CHAIN_FULLSCREEN_DESC>());
    
    if (swapchain_ == nullptr) {
        throw std::runtime_error{"Unable to initialize D3D window."};
    }   
    
    disable_alt_enter(hwnd,device.get_device_from());

    // create everything else
    release_ptr<ID3D11Texture2D> dstexture_ptr_;
    ID3D11RenderTargetView* screenbufferview_ptr_{};
    ID3D11DepthStencilView* dsview_ptr_{};
    ID3D11DepthStencilState* dsstate_ptr_{};

    ID3D11Texture2D* screenbuffer_ptr_{};

    auto rtview_desc = GetDefault<RENDER_TARGET_VIEW_DESC>();

    swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&screenbuffer_ptr_));
    device.get_device_from().CreateRenderTargetView(screenbuffer_ptr_, &rtview_desc, &screenbufferview_ptr_);
    if (screenbufferview_ptr_ == nullptr) {
        throw std::runtime_error{"Unable to initialize D3D window."};
    }

    //screenbuffer_ptr_->Release();
    screenbuffer_.reset(screenbuffer_ptr_);
    
    auto depthstenciltext_desc_ = GetDefault<TEXTURE2D_DESC>();
    auto depthstencil_desc_ = GetDefault<DEPTH_STENCIL_DESC>();
    
    depthstenciltext_desc_.Height = desc.Height;
    depthstenciltext_desc_.Width  = desc.Width;
    depthstenciltext_desc_.Format = DXGI_FORMAT_D32_FLOAT;
    depthstenciltext_desc_.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    depthstenciltext_desc_.SampleDesc = desc.SampleDesc;

    device.get_device_from().CreateTexture2D(&depthstenciltext_desc_, nullptr, &(dstexture_ptr_.expose_ptr()));
    device.get_device_from().CreateDepthStencilView(dstexture_ptr_, nullptr, &dsview_ptr_);
    device.get_device_from().CreateDepthStencilState(&depthstencil_desc_, &dsstate_ptr_);
    
    // TODO : change this to use expose_ptr() interface..

    ////////////////////////////////////////////////////////////////////////////
    // ID Layer stuff, ignore for now..
    // We will be using two render targets: the ordinary display buffer, and a Texture2D 
    // that stores the "id"s (int16s) of whatever is being rendered. We can then sample this to
    // get the identity of whatever it is we are clicking on or mousing over. 
    // ID3D11RenderTargetView* rtargets[] = 
    //    { 
    //      window->rtview_, 
    //	    window->idview_ 
    //    };
    //
    //  device_->OMSetRenderTargets(2, rtargets, NULL);
    ////////////////////////////////////////////////////////////////////////////

    screenbufferview_.reset(screenbufferview_ptr_);        
    depthview_.reset(dsview_ptr_);     
    //depthtexture_.reset(dstexture_ptr_);      
    depthstate_.reset(dsstate_ptr_);

    swapchain_->Present(0,0);

    auto viewport_desc_ = GetDefault<VIEWPORT>();

    viewport_ = viewport_desc_;
    viewport_.Width = _cast<float>(desc.Width);
    viewport_.Height = _cast<float>(desc.Height); 
}

    */
    

}}} // namespaces