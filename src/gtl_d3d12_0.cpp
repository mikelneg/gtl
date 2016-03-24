#include "../include/gtl_d3d12_0.h"

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

    static auto get_dxgi_factory() 
    {  
        release_ptr<IDXGIFactory4> ptr;
        HRESULT result = CreateDXGIFactory2(0, // DXGI_CREATE_FACTORY_DEBUG
                                            __uuidof(decltype(ptr)::type), reinterpret_cast<void**>(&ptr));
        if (win::failed(result)) {
            throw std::runtime_error{__func__};
        }
        return ptr;
    }

    static auto get_adapter()
    {	            
        release_ptr<IDXGIAdapter1> ptr;        
        auto factory = get_dxgi_factory();

        // adapted from:
        // https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/D3D12HelloWorld/src/HelloWindow/DXSample.cpp
        // 
	    for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &ptr); ++i)
	    {
		    DXGI_ADAPTER_DESC1 desc;
		    ptr->GetDesc1(&desc);

		    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { 
		    	ptr.reset();
                continue;
		    }

		    if (win::succeeded(D3D12CreateDevice(ptr.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
		    	return ptr;
		    }
	    }	    

        throw std::runtime_error{__func__}; // if we make it here, we failed
    }

    static auto swchain_desc(HWND hwnd, size_t width, size_t height) 
    { 
        DXGI_SWAP_CHAIN_DESC desc{};        
        desc.BufferDesc.Width = static_cast<UINT>(width);
        desc.BufferDesc.Height = static_cast<UINT>(height);        
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;        
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;        
        desc.OutputWindow = hwnd;        
        desc.BufferCount = 2;        
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
        desc.SampleDesc.Count = 1;        
        desc.Windowed = true;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;            
        return desc;
    }

    static auto swchain_fullscreen_desc()
    {         
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC desc{};
        desc.Windowed = true;
        desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;        
        desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        return desc;
    }

    static auto command_queue_desc() 
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        return desc;
    }

    //////////////////    
    device::device() 
    {         
        HRESULT result = D3D12CreateDevice(nullptr, //adapter.get()
                                           D3D_FEATURE_LEVEL_11_0, 
                                           __uuidof(ID3D12Device),reinterpret_cast<void**>(&dev_ptr));
        if (win::failed(result)) { throw std::runtime_error{__func__}; }
    }    
    
    

    swap_chain::swap_chain(win::window& win, device& dev) 
    {
        RECT client_area{};
        GetClientRect(get_hwnd(win), &client_area);
                                        
        release_ptr<ID3D12CommandQueue> cq_ptr;    
        auto cq_desc = command_queue_desc();

        HRESULT result3 = get(dev).CreateCommandQueue(std::addressof(cq_desc),__uuidof(decltype(cq_ptr)::type),reinterpret_cast<void**>(&cq_ptr));        
        if (win::failed(result3)) {
            throw std::runtime_error{"swap_chain: CreateCommandQueue result not S_OK"};
        }

        auto sc_desc = swchain_desc(get_hwnd(win),width(win),height(win));                
        HRESULT result2 = get_dxgi_factory()->CreateSwapChain(cq_ptr.get(), &sc_desc, &swchain_ptr);        
        if (win::failed(result2)) { 
            throw std::runtime_error{"swap_chain: CreateSwapChainForHwnd result not S_OK"}; 
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
    viewport_.Width = static_cast<float>(desc.Width);
    viewport_.Height = static_cast<float>(desc.Height); 
}

    */
    }

}}} // namespaces