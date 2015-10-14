#include "../include/clist_skybox.h"

#include <gtl/include/win_tools.h>

namespace gtl {
namespace d3d {
namespace command_lists {

void skybox_command_list(d3d::command_list& clist, 
                         d3d::direct_command_allocator& alloc, 
                         d3d::pipeline_state_object& pso, 
                         d3d::cb_root_signature& rsig, 
                         D3D12_VIEWPORT const& viewport,
                         D3D12_RECT const& scissor_rect,
                         d3d::rtv_frame_resources& rtv,
                         d3d::swap_chain& swchain,
                         d3d::rtv_descriptor_heap& rtvheap,
                         d3d::resource& cbuf,            
                         d3d::cbv_descriptor_heap& cbvheap,
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

	clist->RSSetViewports(1, &viewport);
	clist->RSSetScissorRects(1, &scissor_rect);    

	// Indicate that the back buffer will be used as a render target.
	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                        rtv.get_frames()[swchain.get_frame_index()].get(), 
                                        D3D12_RESOURCE_STATE_PRESENT, 
                                        D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvheap->GetCPUDescriptorHandleForHeapStart(), 
                                                static_cast<int>(swchain.get_frame_index()), 
                                                static_cast<UINT>(rtvheap.size()));
	clist->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	clist->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	clist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//clist->IASetVertexBuffers(0, 1, &m_vertexBufferView); // might need something else here.
	//clist->DrawInstanced(3, 1, 0, 0);
    // previous: context_.Draw(14, 0);   
    clist->DrawInstanced(14, 1, 0, 0);
	// Indicate that the back buffer will now be used to present.
	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              rtv.get_frames()[swchain.get_frame_index()].get(), 
                                              D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                              D3D12_RESOURCE_STATE_PRESENT));

    throw_on_fail(clist->Close(),__func__);    
}

}}} // namespaces