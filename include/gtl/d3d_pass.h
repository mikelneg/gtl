#ifndef YIWIBSASSASAFWF_GTL_D3D_PASS_H_
#define YIWIBSASSASAFWF_GTL_D3D_PASS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::d3d::pass

-----------------------------------------------------------------------------*/

#include <gtl/d3d_types.h>

namespace gtl {     
namespace d3d {    

/*
            
    Each pass is distinguished by:

        Pipeline State Object: 
                ...

    Each pass shares the same 

        RootSignature: 
                (e.g.)
                RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),         
                CBV(b0),                                           
                DescriptorTable( Sampler(s0) ),                    
                DescriptorTable( SRV(t0, numDescriptors = 2),      
                                 UAV(u0) ),                        
                RootConstants(num32BitConstants = 8, b0, space = 1)      
  

    class pass {
    
        gtl::d3d::root_signature root_sig_;
        gtl::d3d::pipeline_state_object pso_;
    
    public:
        
        template <typename CommandList, typename RtvHeap>
        void draw(CommandList& cl, 
                  CBVSRVUAVHeap,
                  SamplerHeap,
                  RTVHeap,
                  DSVHeap)           
        {
            
            cl->SetGraphicsRootSignature(root_sig_.get());
            auto heaps = { sampler_heap_.get(), resource_heap_.get() };
        	cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin()); // all do this..sort of

            
            // these depend on the root signature.. 
            //  ||                                // these depend on the particular pass
            //  ||                                //      ||
            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
            //                                    // correctness depends on whatever supplies the dheap..

            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, resource_heap_->GetGPUDescriptorHandleForHeapStart());

            // every drawcall sets viewports and such..
            auto viewports = { std::addressof(viewport_) };
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
            cl->RSSetScissorRects(1, std::addressof(scissor_));            
            
            // has to be specified 
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            
            // determine which rtv to render to..
            // 
            // CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};
            //  rtv_handle.Offset(idx, rtv_heap_.increment_value());
            
            float clearvalue[]{0.0f,0.0f,0.0f,0.0f};            

            // doesn't matter..
            cl->ClearRenderTargetView(rtv_handle,clearvalue,1,&scissor_); 
                               
            // depends on the layout of the descriptor handles in the table we have bound..
            // if I use a multi-render target approach with double buffering it is probably better to 
            // stagger descriptors continously, so something like:
            //
            //      color_outputrtv =   c[1,2,3]
            //      normal_outputrtv =  n[1,2,3]
            //      whatever_output =   w[1,2,3]                         
            //      id_layeroutput =    i[1,2,3]
            //
            //      rtv_outs = [c1,n1,w1,i1] descriptor heaps
            //                 [c2,n2,w2,i2]
            //
            //      ... may be a downside.. 
            //
            //  "singlehandletodescriptorrange" == true or false

            // determined from outside.. 

            bind(cl,rtv_heap);


            cl->OMSetRenderTargets(1, &rtv_handle, TRUE, nullptr);
            cl->DrawInstanced(14, 1, 0, 0);             

            // everyone..
            cl->Close();
            
                    
            // ..........
            // so I need some wrappers.. e.g., around the rtvtargets (single/non-single handle)    




        }

    };
*/
}} // namespaces
#endif

