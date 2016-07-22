#ifndef NSNLLSLSFW_GTL_D3D_SELECT_IMPLEMENTATION_H_
#define NSNLLSLSFW_GTL_D3D_SELECT_IMPLEMENTATION_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                      
    Specifies default implementation in gtl::d3d

    Does not need to be included directly 
-----------------------------------------------------------------------------*/

//#include <Windows.h>
#include <d3d12.h> 
#include <d3dx12.h> // not supplied by the sdk currently
#include <dxgi1_4.h>

namespace gtl {    
namespace d3d {      

namespace version_12_0 {
    namespace raw {
        using Object              = ID3D12Object;
        using Factory             = IDXGIFactory4;
        using Device              = ID3D12Device;
        using DebugDevice         = ID3D12DebugDevice;
        using SwapChain           = IDXGISwapChain3;
        using Adapter             = IDXGIAdapter1;
        using AdapterDesc         = DXGI_ADAPTER_DESC1;
        using CommandQueue        = ID3D12CommandQueue;
        using Format              = DXGI_FORMAT;
        using DescriptorHeap      = ID3D12DescriptorHeap;
        using DescriptorHeapDesc  = D3D12_DESCRIPTOR_HEAP_DESC;
        using Resource            = ID3D12Resource;     
        using ResourceDesc        = D3D12_RESOURCE_DESC;
        using ResourceBarrier     = D3D12_RESOURCE_BARRIER;
        using CommandAllocator    = ID3D12CommandAllocator;
        using RootSignature       = ID3D12RootSignature;
        using Blob                = ID3DBlob;
        using PipelineState       = ID3D12PipelineState;        
        using Debug               = ID3D12Debug;
        using GraphicsCommandList = ID3D12GraphicsCommandList;
        using Fence               = ID3D12Fence;
        using Viewport            = D3D12_VIEWPORT;
        using ScissorRect         = D3D12_RECT;         
        using CommandList         = ID3D12CommandList;
        using CpuDescriptorHandle = D3D12_CPU_DESCRIPTOR_HANDLE;        
        using PresentParameters   = DXGI_PRESENT_PARAMETERS;        
        using SwapChainDesc       = DXGI_SWAP_CHAIN_DESC;        
        using SamplerDesc         = D3D12_SAMPLER_DESC;
        using CommandQueueDesc    = D3D12_COMMAND_QUEUE_DESC;
        using SrvDesc             = D3D12_SHADER_RESOURCE_VIEW_DESC;  
        using DepthStencilDesc    = D3D12_DEPTH_STENCIL_DESC;
        using DsvDesc             = D3D12_DEPTH_STENCIL_VIEW_DESC;
        using VertexBufferView    = D3D12_VERTEX_BUFFER_VIEW;
        using StreamOutputDesc    = D3D12_STREAM_OUTPUT_DESC;
        using RasterizerDesc      = D3D12_RASTERIZER_DESC;
        using BlendDesc           = D3D12_BLEND_DESC;
        using HeapProperties      = D3D12_HEAP_PROPERTIES;
        using TextureLayout       = D3D12_TEXTURE_LAYOUT;
        using SwapChainFullscreenDesc   = DXGI_SWAP_CHAIN_FULLSCREEN_DESC;        
        using ComputePipelineStateDesc  = D3D12_COMPUTE_PIPELINE_STATE_DESC;
        using ConstantBufferViewDesc    = D3D12_CONSTANT_BUFFER_VIEW_DESC;
        using GraphicsPipelineStateDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC;        
        using RenderTargetViewDesc      = D3D12_RENDER_TARGET_VIEW_DESC;
        using UnorderedAccessViewDesc   = D3D12_UNORDERED_ACCESS_VIEW_DESC;

        namespace cx {
            using CpuDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE;
            using HeapProperties      = CD3DX12_HEAP_PROPERTIES;
            using RasterizerDesc      = CD3DX12_RASTERIZER_DESC;
            using BlendDesc           = CD3DX12_BLEND_DESC;
            using ResourceDesc        = CD3DX12_RESOURCE_DESC;
            using DepthStencilDesc    = CD3DX12_DEPTH_STENCIL_DESC;            
        }        
    }     
}         

}} // namespaces
#endif
