/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/d3d_helper_funcs.h"
#include <gtl/d3d_types.h>

#include <gtl/intrusive_ptr.h>
#include <gtl/win_tools.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <windows.h>
#include <winerror.h>
//#include <ostream>
//#include <gtl/d3d_ostream.h>

namespace gtl {
namespace d3d {
    namespace version_12_0 {

        namespace { // internal helpers

            release_ptr<raw::Factory> get_dxgi_factory()
            {
                release_ptr<raw::Factory> ptr;
                win::throw_on_fail(CreateDXGIFactory2(0, __uuidof(raw::Factory), expose_as_void_pp(ptr)), __func__);
                return ptr;
            }

            void disable_alt_enter(HWND hwnd)
            {
                auto factory = get_dxgi_factory();
                win::throw_on_fail(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER), __func__);
            }
        }

        ////////////////////////////////////////////////////////

        std::vector<raw::AdapterDesc> enumerate_adaptors()
        {
            // adapted from https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/D3D12HelloWorld/src/HelloWindow/DXSample.cpp
            auto factory = get_dxgi_factory();
            release_ptr<raw::Adapter> ptr;
            std::vector<raw::AdapterDesc> adapters;
            for (unsigned i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, std::addressof(ptr.expose_ptr())); ++i)
            {
                raw::AdapterDesc desc{};
                ptr->GetDesc1(std::addressof(desc));
                adapters.emplace_back(desc);
            }

            return adapters;
        }

        //raw::SwapChainDesc create_swapchain_desc(tags::flipmodel_windowed, HWND hwnd, unsigned num_buffers, unsigned width, unsigned height)
        //{
        //    if (num_buffers < 2 || num_buffers > 16) { throw std::logic_error{__func__}; }
        //
        //    raw::SwapChainDesc desc{};
        //    desc.BufferDesc.Width = width;
        //    desc.BufferDesc.Height = height;   // Rumor suggests this must be a multiple of 4..
        //    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        //    desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        //    //desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
        //    desc.OutputWindow = hwnd;
        //    desc.BufferCount = num_buffers;    // Required to be 2-16
        //    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        //    desc.SampleDesc.Count = 1;         // Required for flip-chain
        //    desc.SampleDesc.Quality = 0;       // Required for flip-chain
        //    desc.Windowed = true;
        //    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // or _SEQUENTIAL --
        //    desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        //    return desc;
        //}

        void report_live_objects(device& dev)
        {
            gtl::d3d::release_ptr<raw::DebugDevice> debug_device;
            win::throw_on_fail(dev->QueryInterface(__uuidof(raw::DebugDevice), expose_as_void_pp(debug_device)), __func__);
            debug_device->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
        }

        void wait_for_gpu(device& dev, command_queue& cqueue_)
        {
            gtl::d3d::fence fence_{dev};
            fence_.synchronized_increment(cqueue_);
            //gtl::win::waitable_handle event_;
            //auto current_value = fence_->GetCompletedValue();
            //win::throw_on_fail(fence_->SetEventOnCompletion(current_value + 1, event_)
            //                   ,__func__);
            //win::throw_on_fail(cqueue_->Signal(fence_.get(),current_value + 1)
            //                   ,__func__);
            //wait(event_);
        }

        void wait_for_gpu(command_queue& cqueue_)
        {
            wait_for_gpu(get_device_from(cqueue_), cqueue_);
        }

        //release_ptr<raw::Blob> dummy_rootsig_1()
        //{
        //    std::vector<CD3DX12_DESCRIPTOR_RANGE> table1_, table2_;
        //	std::vector<CD3DX12_ROOT_PARAMETER> params_;
        //
        //    table1_.resize(1);
        //    table2_.resize(3);
        //
        //    table1_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);   // 1 descriptor, register 0
        //
        //    table2_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		  // 1 descriptor, register 0
        //    table2_[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);    // 1 desc, reg 0, space 1
        //    table2_[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);       // 1 desc, reg 0
        //
        //    params_.resize(3);
        //    params_[0].InitAsConstantBufferView(0);
        //    params_[1].InitAsDescriptorTable(win::extent(table1_), table1_.data(), D3D12_SHADER_VISIBILITY_PIXEL);
        //    params_[2].InitAsDescriptorTable(win::extent(table2_), table2_.data(), D3D12_SHADER_VISIBILITY_ALL);
        //
        //	D3D12_ROOT_SIGNATURE_FLAGS flags_ =
        //		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        //
        //	CD3DX12_ROOT_SIGNATURE_DESC desc;
        //	desc.Init(win::extent(params_), params_.data(), 0, nullptr, flags_);
        //
        //    release_ptr<raw::Blob> signature_;
        //    release_ptr<raw::Blob> error_; // not currently using
        //    win::throw_on_fail(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
        //                                              &(signature_.expose_ptr()), &(error_.expose_ptr()))
        //                                              ,__func__);
        //    return signature_;
        //}
        //
        //release_ptr<raw::Blob> dummy_rootsig_2()
        //{
        //    std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
        //	std::vector<CD3DX12_ROOT_PARAMETER> params_;
        //
        //    ranges.resize(3);
        //    params_.resize(2);
        //
        //    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        //    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        //    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
        //    params_[0].InitAsDescriptorTable(2, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        //    params_[1].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);
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
        //                            flags_);
        //
        //    release_ptr<raw::Blob> signature;
        //    release_ptr<raw::Blob> error;
        //
        //	win::throw_on_fail(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &expose(signature), &expose(error))
        //                  ,__func__);
        //
        //    return signature;
        //}
        //
        //release_ptr<raw::Blob> dummy_rootsig_3()
        //{
        //    std::vector<CD3DX12_DESCRIPTOR_RANGE> table1_, table2_;
        //	std::vector<CD3DX12_ROOT_PARAMETER> params_;
        //
        //    table1_.resize(1);
        //    table2_.resize(3);
        //
        //    table1_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);   // 1 descriptor, register 0
        //
        //    table2_[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);		  // 1 descriptor, register 0
        //    table2_[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);    // 1 desc, reg 0, space 1
        //    table2_[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);       // 1 desc, reg 0
        //
        //    params_.resize(4);
        //    params_[0].InitAsConstantBufferView(0);
        //    params_[1].InitAsDescriptorTable(win::extent(table1_), table1_.data(), D3D12_SHADER_VISIBILITY_PIXEL);
        //    params_[2].InitAsDescriptorTable(win::extent(table2_), table2_.data(), D3D12_SHADER_VISIBILITY_ALL);
        //    params_[3].InitAsConstants(8,0,1);
        //
        //	D3D12_ROOT_SIGNATURE_FLAGS flags_ =
        //		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        //		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        //
        //	CD3DX12_ROOT_SIGNATURE_DESC desc;
        //	desc.Init(win::extent(params_), params_.data(), 0, nullptr, flags_);
        //
        //    release_ptr<raw::Blob> signature_;
        //    release_ptr<raw::Blob> error_; // not currently using
        //    win::throw_on_fail(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
        //                                              &(signature_.expose_ptr()), &(error_.expose_ptr()))
        //                                              ,__func__);
        //    return signature_;
        //}
        //
        //

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
    }
}
} // namespaces
