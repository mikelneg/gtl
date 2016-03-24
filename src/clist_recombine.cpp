#include "../include/clist_recombine.h"

#include <gtl/include/win_tools.h>

namespace gtl {
namespace d3d {
namespace command_lists {


void recombine_command_list(d3d::command_list& clist, 
                         d3d::direct_command_allocator& alloc, 
                         d3d::pipeline_state_object& pso, 
                         d3d::cb_root_signature& rsig, 
                         std::initializer_list<D3D12_VIEWPORT*> viewports,                         
                         D3D12_RECT const& scissor_rect,                         
    //d3d::swapchain_rtv_heap& rtv,
                         d3d::resource& target_resource,
                         //d3d::rtv_descriptor_heap& rtv_heap,
                         d3d::rtv_srv_texture2D& rtv_srv_,                         
                         d3d::resource& cbuf,            
                         d3d::resource_descriptor_heap& cbvheap,
                         d3d::sampler_descriptor_heap& smpheap)
{    
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
    throw_on_fail(alloc->Reset(),__func__);

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	throw_on_fail(clist->Reset(alloc.get(), pso.get()),__func__);

	// Set necessary state.
	clist->SetGraphicsRootSignature(rsig.get());
    
    ID3D12DescriptorHeap* ppHeaps[] = { cbvheap.get(), smpheap.get() };
	clist->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        
    clist->SetGraphicsRootConstantBufferView(0, cbuf.get()->GetGPUVirtualAddress());
    
    clist->SetGraphicsRootDescriptorTable(1, smpheap.get()->GetGPUDescriptorHandleForHeapStart());
    clist->SetGraphicsRootDescriptorTable(2, cbvheap.get()->GetGPUDescriptorHandleForHeapStart());

    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          target_resource.get(), 
                                          D3D12_RESOURCE_STATE_PRESENT, 
                                          D3D12_RESOURCE_STATE_COPY_DEST,
                                          D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                          D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));    
	
    clist->RSSetViewports(array_size(viewports), *viewports.begin());
	clist->RSSetScissorRects(1, &scissor_rect);    

	// Indicate that the back buffer will be used as a render target.
	//clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //                                    target_resource.get(), 
    //                                    D3D12_RESOURCE_STATE_PRESENT, 
    //                                    D3D12_RESOURCE_STATE_RENDER_TARGET));    
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap->GetCPUDescriptorHandleForHeapStart(), 
    //                                            static_cast<int>(swchain.get_frame_index()), 
    //                                            static_cast<UINT>(rtvheap.size()));	

	// Record commands.
	//const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	//clist->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	clist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//clist->IASetVertexBuffers(0, 1, &m_vertexBufferView); // might need something else here.
	//clist->DrawInstanced(3, 1, 0, 0);
    // previous: context_.Draw(14, 0);   
    
    clist->OMSetRenderTargets(2, &rtv_srv_.rtv_heap()->GetCPUDescriptorHandleForHeapStart(), TRUE, nullptr);    
    clist->DrawInstanced(14, 1, 0, 0);

    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          rtv_srv_.get(), 
                                          D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE));
    
    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          target_resource.get(), 
                                          D3D12_RESOURCE_STATE_PRESENT, 
                                          D3D12_RESOURCE_STATE_COPY_DEST,
                                          D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                          D3D12_RESOURCE_BARRIER_FLAG_END_ONLY));    
    
    //CD3DX12_SUBRESOURCE_FOOTPRINT footprint{DXGI_FORMAT_R8
    ////clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
    //CD3DX12_TEXTURE_COPY_LOCATION dest{target_resource.get(), 0};
    //CD3DX12_TEXTURE_COPY_LOCATION src{rtv_srv_.get(), 0};
    //CD3DX12_TEXTURE_COPY_LOCATION srca{rtv_srv_.get(), 1};
    ////CmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
    //
    //clist->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
    //clist->CopyTextureRegion(&dest, 400, 0, 0, &srca, nullptr);
    
    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          rtv_srv_.get(), 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE,                                  
                                          D3D12_RESOURCE_STATE_RENDER_TARGET));    

	// Indicate that the back buffer will now be used to present.
	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              target_resource.get(),
                                              D3D12_RESOURCE_STATE_COPY_DEST, 
                                              D3D12_RESOURCE_STATE_PRESENT));

    throw_on_fail(clist->Close(),__func__);    
}

}}} // namespaces