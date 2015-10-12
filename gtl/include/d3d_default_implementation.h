#ifndef NSNLLSLSFW_GTL_D3D_SELECT_IMPLEMENTATION_H_
#define NSNLLSLSFW_GTL_D3D_SELECT_IMPLEMENTATION_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                      
    Specifies default implementation in gtl::d3d

    Does not need to be included directly 
-----------------------------------------------------------------------------*/

#include <d3d12.h> 
#include <d3dx12.h> // not supplied by the sdk currently
#include <dxgi1_4.h>

namespace   gtl {    
namespace   d3d {    

    namespace _12_0 {
        using DXGIFactory              = IDXGIFactory4;
        using D3D12Device              = ID3D12Device;
        using DXGISwapChain            = IDXGISwapChain3;
        using DXGIAdapter              = IDXGIAdapter1;
        using D3D12CommandQueue        = ID3D12CommandQueue;
        using D3D12DescriptorHeap      = ID3D12DescriptorHeap;
        using D3D12Resource            = ID3D12Resource;              
        using D3D12CommandAllocator    = ID3D12CommandAllocator;
        using D3D12RootSignature       = ID3D12RootSignature;
        using D3DBlob                  = ID3DBlob;
        using D3D12PipelineState       = ID3D12PipelineState;
        using D3D12Debug               = ID3D12Debug;
        using D3D12GraphicsCommandList = ID3D12GraphicsCommandList;
        using D3D12Fence               = ID3D12Fence;
        using D3D12Viewport            = D3D12_VIEWPORT;
        using D3D12ScissorRect         = D3D12_RECT;

        constexpr int frame_count() noexcept { return 2; }    
    }
          
namespace default = _12_0;
using namespace default;
}} // namespaces
#endif
