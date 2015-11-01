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
        desc.OutputWindow = hwnd;        
        desc.BufferCount = num_buffers;    // Required to be 2-16
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
        desc.SampleDesc.Count = 1;         // Required for flip-chain       
        desc.SampleDesc.Quality = 0;       // Required for flip-chain
        desc.Windowed = true;        
        //desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Can't figure out which provides independent flip.. 
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
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
        //auto value = fence_->GetCompletedValue() + 1;
        //fence_->Signal(value);
        //HRESULT result = cqueue_->Wait(fence_.get(), value);
        //win::throw_on_fail(result,__func__);
    }

     /*               
            

        
    swapchain_ = CreateSwapChainForHwnd(hwnd, device.get_device(), desc, GetDefault<SWAP_CHAIN_FULLSCREEN_DESC>());
    
    if (swapchain_ == nullptr) {
        throw std::runtime_error{"Unable to initialize D3D window."};
    }   
    
    disable_alt_enter(hwnd,device.get_device());

    // create everything else
    release_ptr<ID3D11Texture2D> dstexture_ptr_;
    ID3D11RenderTargetView* screenbufferview_ptr_{};
    ID3D11DepthStencilView* dsview_ptr_{};
    ID3D11DepthStencilState* dsstate_ptr_{};

    ID3D11Texture2D* screenbuffer_ptr_{};

    auto rtview_desc = GetDefault<RENDER_TARGET_VIEW_DESC>();

    swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&screenbuffer_ptr_));
    device.get_device().CreateRenderTargetView(screenbuffer_ptr_, &rtview_desc, &screenbufferview_ptr_);
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

    device.get_device().CreateTexture2D(&depthstenciltext_desc_, nullptr, &(dstexture_ptr_.expose_ptr()));
    device.get_device().CreateDepthStencilView(dstexture_ptr_, nullptr, &dsview_ptr_);
    device.get_device().CreateDepthStencilState(&depthstencil_desc_, &dsstate_ptr_);
    
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