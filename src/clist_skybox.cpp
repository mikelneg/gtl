#include "gtl/clist_skybox.h"

#include <gtl/win_tools.h>

namespace gtl {
namespace d3d {
namespace graphics_command_lists {

static 
void play_skybox_bundle(d3d::graphics_command_list& clist) {




}


void skybox_graphics_command_list(d3d::graphics_command_list& clist, 
                         d3d::direct_command_allocator& alloc, 
                         d3d::pipeline_state_object& pso, 
                         d3d::root_signature& rsig, 
                         std::initializer_list<D3D12_VIEWPORT*> viewports,                         
                         D3D12_RECT const& scissor_rect,                         
    //d3d::swapchain_rtv_heap_& rtv,
                         d3d::resource& target_resource,
                         //d3d::rtv_descriptor_heap& rtv_heap_,
                         d3d::rtv_srv_texture2D& rtv_srv_,                         
                         d3d::resource& cbuf,            
                         d3d::resource_descriptor_heap& resource_heap,
                         d3d::sampler_descriptor_heap& sampler_heap)
{    
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
    win::throw_on_fail(alloc->Reset(),__func__);

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	win::throw_on_fail(clist->Reset(alloc.get(), pso.get()),__func__);

	// Set necessary state.
	clist->SetGraphicsRootSignature(rsig.get());
    
    ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap.get(), resource_heap.get() };
	clist->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        
    clist->SetGraphicsRootConstantBufferView(0, cbuf.get()->GetGPUVirtualAddress());
    
    clist->SetGraphicsRootDescriptorTable(1, sampler_heap.get()->GetGPUDescriptorHandleForHeapStart());
    clist->SetGraphicsRootDescriptorTable(2, resource_heap.get()->GetGPUDescriptorHandleForHeapStart());

    // Split barriers currently cause the diagnostic tool to choke..

    //clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //                                      target_resource.get(), 
    //                                      D3D12_RESOURCE_STATE_PRESENT, 
    //                                      D3D12_RESOURCE_STATE_COPY_DEST,
    //                                      D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    //                                      D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));    
	
    clist->RSSetViewports(win::array_size(viewports), *viewports.begin());
	clist->RSSetScissorRects(1, &scissor_rect);    

	// Indicate that the back buffer will be used as a render target.
	//clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //                                    target_resource.get(), 
    //                                    D3D12_RESOURCE_STATE_PRESENT, 
    //                                    D3D12_RESOURCE_STATE_RENDER_TARGET));    
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap_->GetCPUDescriptorHandleForHeapStart(), 
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
                                          D3D12_RESOURCE_STATE_COPY_DEST));
                                          //D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                          //D3D12_RESOURCE_BARRIER_FLAG_END_ONLY));    

    CD3DX12_SUBRESOURCE_FOOTPRINT fdest{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
    CD3DX12_SUBRESOURCE_FOOTPRINT fsrc{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
    
    //UINT64 Offset;
    //D3D12_SUBRESOURCE_FOOTPRINT Footprint;    
    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_dest{0,fdest};
    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_src{0,fdest};
    //clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
    D3D12_TEXTURE_COPY_LOCATION dest{target_resource.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    dest.SubresourceIndex = 0;

     auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };

    CD3DX12_BOX box_a{0,0,200,134};
    CD3DX12_BOX box_b{0,0,200,134};
    
    //ID3D12Resource *pResource;
    //D3D12_TEXTURE_COPY_TYPE Type;
    //union 
    //    {
    //    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
    //    UINT SubresourceIndex;
    //    } 	;
    //} 	D3D12_TEXTURE_COPY_LOCATION;

    //CD3DX12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(),1};

    D3D12_TEXTURE_COPY_LOCATION bsrc{rtv_srv_.get(),D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    D3D12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    bsrc.SubresourceIndex = 0;//calc(0,0,0,1,2);
    bsrc_a.SubresourceIndex = 1;//calc(0,0,1,1,2);
    //CmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        
    clist->CopyTextureRegion(&dest, 0, 0, 0, &bsrc_a, &box_a);
    clist->CopyTextureRegion(&dest, 200, 0, 0, &bsrc, &box_b);
    
    //clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
    
    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          rtv_srv_.get(), 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE,                                  
                                          D3D12_RESOURCE_STATE_RENDER_TARGET));    

	// Indicate that the back buffer will now be used to present.
	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              target_resource.get(),
                                              D3D12_RESOURCE_STATE_COPY_DEST, 
                                              D3D12_RESOURCE_STATE_PRESENT));

    win::throw_on_fail(clist->Close(),__func__);    
}

//void cs_list(d3d::graphics_command_list& clist, 
//             d3d::resource& target_resource,
//             d3d::rtv_srv_texture2D& rtv_srv_)
//{    
//    
//    ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap.get(), resource_heap.get() };
//	clist->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        
//    clist->SetGraphicsRootConstantBufferView(0, cbuf.get()->GetGPUVirtualAddress());
//    
//    clist->SetGraphicsRootDescriptorTable(1, sampler_heap.get()->GetGPUDescriptorHandleForHeapStart());
//    clist->SetGraphicsRootDescriptorTable(2, resource_heap.get()->GetGPUDescriptorHandleForHeapStart());
//
//    // Split barriers currently cause the diagnostic tool to choke..
//
//    //clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//    //                                      target_resource.get(), 
//    //                                      D3D12_RESOURCE_STATE_PRESENT, 
//    //                                      D3D12_RESOURCE_STATE_COPY_DEST,
//    //                                      D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
//    //                                      D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));    
//	
//    clist->RSSetViewports(win::array_size(viewports), *viewports.begin());
//	clist->RSSetScissorRects(1, &scissor_rect);    
//
//	// Indicate that the back buffer will be used as a render target.
//	//clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//    //                                    target_resource.get(), 
//    //                                    D3D12_RESOURCE_STATE_PRESENT, 
//    //                                    D3D12_RESOURCE_STATE_RENDER_TARGET));    
//	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap_->GetCPUDescriptorHandleForHeapStart(), 
//    //                                            static_cast<int>(swchain.get_frame_index()), 
//    //                                            static_cast<UINT>(rtvheap.size()));	
//
//	// Record commands.
//	//const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
//	//clist->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//	clist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	//clist->IASetVertexBuffers(0, 1, &m_vertexBufferView); // might need something else here.
//	//clist->DrawInstanced(3, 1, 0, 0);
//    // previous: context_.Draw(14, 0);   
//    
//    clist->OMSetRenderTargets(2, &rtv_srv_.rtv_heap_()->GetCPUDescriptorHandleForHeapStart(), TRUE, nullptr);    
//    clist->DrawInstanced(14, 1, 0, 0);
//
//    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                          rtv_srv_.get(), 
//                                          D3D12_RESOURCE_STATE_RENDER_TARGET, 
//                                          D3D12_RESOURCE_STATE_COPY_SOURCE));
//    
//    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                          target_resource.get(), 
//                                          D3D12_RESOURCE_STATE_PRESENT, 
//                                          D3D12_RESOURCE_STATE_COPY_DEST));
//                                          //D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
//                                          //D3D12_RESOURCE_BARRIER_FLAG_END_ONLY));    
//
//    CD3DX12_SUBRESOURCE_FOOTPRINT fdest{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
//    CD3DX12_SUBRESOURCE_FOOTPRINT fsrc{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
//    
//    //UINT64 Offset;
//    //D3D12_SUBRESOURCE_FOOTPRINT Footprint;    
//    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_dest{0,fdest};
//    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_src{0,fdest};
//    //clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
//    D3D12_TEXTURE_COPY_LOCATION dest{target_resource.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
//    dest.SubresourceIndex = 0;
//
//     auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
//                        { 
//                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
//                        };
//
//    CD3DX12_BOX box_a{0,0,200,134};
//    CD3DX12_BOX box_b{0,0,200,134};
//    
//    //ID3D12Resource *pResource;
//    //D3D12_TEXTURE_COPY_TYPE Type;
//    //union 
//    //    {
//    //    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
//    //    UINT SubresourceIndex;
//    //    } 	;
//    //} 	D3D12_TEXTURE_COPY_LOCATION;
//
//    //CD3DX12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(),1};
//
//    D3D12_TEXTURE_COPY_LOCATION bsrc{rtv_srv_.get(),D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
//    D3D12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
//    bsrc.SubresourceIndex = 0;//calc(0,0,0,1,2);
//    bsrc_a.SubresourceIndex = 1;//calc(0,0,1,1,2);
//    //CmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
//        
//    clist->CopyTextureRegion(&dest, 0, 0, 0, &bsrc_a, &box_a);
//    clist->CopyTextureRegion(&dest, 200, 0, 0, &bsrc, &box_b);
//    
//    //clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
//    
//    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                          rtv_srv_.get(), 
//                                          D3D12_RESOURCE_STATE_COPY_SOURCE,                                  
//                                          D3D12_RESOURCE_STATE_RENDER_TARGET));    
//
//	// Indicate that the back buffer will now be used to present.
//	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                                              target_resource.get(),
//                                              D3D12_RESOURCE_STATE_COPY_DEST, 
//                                              D3D12_RESOURCE_STATE_PRESENT));
//
//    win::throw_on_fail(clist->Close(),__func__);    
//}


//void skybox_graphics_command_list_first(d3d::graphics_command_list& clist,                                                          
//                         d3d::cb_root_signature& rsig,                         
//                         std::initializer_list<D3D12_VIEWPORT*> viewports,                         
//                         D3D12_RECT const& scissor_rect,                         
//    //d3d::swapchain_rtv_heap_& rtv,
//                         d3d::resource& target_resource,
//                         //d3d::rtv_descriptor_heap& rtv_heap_,
//                         d3d::uav_texture2D& rtv_srv_,                         
//                         d3d::resource& cbuf,            
//                         d3d::resource_descriptor_heap& resource_heap,
//                         d3d::sampler_descriptor_heap& sampler_heap)
//{    
//	// Command list allocators can only be reset when the associated 
//	// command lists have finished execution on the GPU; apps should use 
//	// fences to determine GPU execution progress.
//
//	// Set necessary state.
//	clist->SetGraphicsRootSignature(rsig.get());
//    
//    ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap.get(), resource_heap.get() };
//	clist->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        
//    clist->SetGraphicsRootConstantBufferView(0, cbuf.get()->GetGPUVirtualAddress());
//    
//    clist->SetGraphicsRootDescriptorTable(1, sampler_heap.get()->GetGPUDescriptorHandleForHeapStart());
//    clist->SetGraphicsRootDescriptorTable(2, resource_heap.get()->GetGPUDescriptorHandleForHeapStart());
//
//    // Split barriers currently cause the diagnostic tool to choke..
//
//    //clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//    //                                      target_resource.get(), 
//    //                                      D3D12_RESOURCE_STATE_PRESENT, 
//    //                                      D3D12_RESOURCE_STATE_COPY_DEST,
//    //                                      D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
//    //                                      D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));    
//	
//    clist->RSSetViewports(win::array_size(viewports), *viewports.begin());
//	clist->RSSetScissorRects(1, &scissor_rect);    
//
//	// Indicate that the back buffer will be used as a render target.
//	//clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//    //                                    target_resource.get(), 
//    //                                    D3D12_RESOURCE_STATE_PRESENT, 
//    //                                    D3D12_RESOURCE_STATE_RENDER_TARGET));    
//	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap_->GetCPUDescriptorHandleForHeapStart(), 
//    //                                            static_cast<int>(swchain.get_frame_index()), 
//    //                                            static_cast<UINT>(rtvheap.size()));	
//
//	// Record commands.
//	//const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
//	//clist->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//	clist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	//clist->IASetVertexBuffers(0, 1, &m_vertexBufferView); // might need something else here.
//	//clist->DrawInstanced(3, 1, 0, 0);
//    // previous: context_.Draw(14, 0);   
//    
//    clist->OMSetRenderTargets(2, &rtv_srv_.rtv_heap_()->GetCPUDescriptorHandleForHeapStart(), TRUE, nullptr);    
//    clist->DrawInstanced(14, 1, 0, 0);
//
//}


void skybox_graphics_command_list_second(d3d::graphics_command_list& clist,                                 
                                         d3d::resource& target_resource,                         
                                          d3d::uav_texture2D& rtv_srv_)
{             
    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          target_resource.get(), 
                                          D3D12_RESOURCE_STATE_PRESENT, 
                                          D3D12_RESOURCE_STATE_COPY_DEST));
                                          //D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                          //D3D12_RESOURCE_BARRIER_FLAG_END_ONLY));    

    CD3DX12_SUBRESOURCE_FOOTPRINT fdest{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
    CD3DX12_SUBRESOURCE_FOOTPRINT fsrc{DXGI_FORMAT_R8G8B8A8_UNORM,300,200,1,300};
    
    //UINT64 Offset;
    //D3D12_SUBRESOURCE_FOOTPRINT Footprint;    
    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_dest{0,fdest};
    //D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_src{0,fdest};
    //clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
    D3D12_TEXTURE_COPY_LOCATION dest{target_resource.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    dest.SubresourceIndex = 0;

     auto calc = []( UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize )
                        { 
                            return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize; 
                        };

    CD3DX12_BOX box_a{0,0,200,134};
    CD3DX12_BOX box_b{0,0,200,134};
    
    //ID3D12Resource *pResource;
    //D3D12_TEXTURE_COPY_TYPE Type;
    //union 
    //    {
    //    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
    //    UINT SubresourceIndex;
    //    } 	;
    //} 	D3D12_TEXTURE_COPY_LOCATION;

    //CD3DX12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(),1};

    D3D12_TEXTURE_COPY_LOCATION bsrc{rtv_srv_.get(),D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    D3D12_TEXTURE_COPY_LOCATION bsrc_a{rtv_srv_.get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    bsrc.SubresourceIndex = 0;//calc(0,0,0,1,2);
    bsrc_a.SubresourceIndex = 1;//calc(0,0,1,1,2);
    //CmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        
    //clist->CopyTextureRegion(&dest, 0, 0, 0, &bsrc_a, &box_a);
    //clist->CopyTextureRegion(&dest, 200, 0, 0, &bsrc, &box_b);    
    clist->CopyResource(target_resource.get(), rtv_srv_.get()); 
    
    clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          rtv_srv_.get(), 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE,                                  
                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS));    

	// Indicate that the back buffer will now be used to present.
	clist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                              target_resource.get(),
                                              D3D12_RESOURCE_STATE_COPY_DEST, 
                                              D3D12_RESOURCE_STATE_PRESENT));

    win::throw_on_fail(clist->Close(),__func__);    
}



}}} // namespaces