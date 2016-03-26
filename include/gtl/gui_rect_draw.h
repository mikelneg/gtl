#ifndef UWAAFVBBZZASASDFF_GTL_GUI_RECT_DRAW_H_
#define UWAAFVBBZZASASDFF_GTL_GUI_RECT_DRAW_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::gui::detail
    
    class rect_draw
-----------------------------------------------------------------------------*/

#include <gtl/d3d_types.h>
#include <gtl/gui_rect.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

namespace gtl {
namespace d3d {

    class rect_draw {        

        struct Vertex {
            Eigen::Vector4f position;
            Eigen::Vector4f uv;        
        };

        static constexpr unsigned MAX_RECTS = 40 * 4;

        template <typename T>
        using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;        

        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;        

        gtl::d3d::resource_descriptor_heap vbuffer_descriptors_;        
        aligned_vector<Vertex> mutable mesh_;                
        std::array<gtl::d3d::constant_buffer,3> mutable vbuffers_;        
        
        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;        
        gtl::d3d::srv texture_;               
    
        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::pipeline_state_object pso_;    
                
        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;

        auto vertex_layout() {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
            };                         

        }

        auto pso_desc(gtl::d3d::device& dev, gtl::d3d::root_signature& rsig, gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
		    desc_.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
		    desc_.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		    desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		                            

            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  
            desc_.RasterizerState.DepthClipEnable = false;
            desc_.RasterizerState.AntialiasedLineEnable = false;            

            desc_.InputLayout.pInputElementDescs = &layout_[0];
            desc_.InputLayout.NumElements = static_cast<unsigned>(layout_.size());
            
                D3D12_BLEND_DESC blend_desc_ = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                  
                blend_desc_.RenderTarget[0].BlendEnable = true;
                blend_desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
                blend_desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                blend_desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
                blend_desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;                
                blend_desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;                 
                blend_desc_.RenderTarget[0].BlendEnable = true;
       
            desc_.BlendState = blend_desc_;                                  
            
            desc_.DepthStencilState.DepthEnable = FALSE;
		    desc_.DepthStencilState.StencilEnable = FALSE;
		    desc_.SampleMask = UINT_MAX;                        
		    desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;            
		    desc_.NumRenderTargets = 1;
		    desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;            
		    desc_.SampleDesc.Count = 1;              
            return desc_;		    
        }

        auto sampler_desc() {
            D3D12_SAMPLER_DESC sampler_{};
            sampler_.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;            
            sampler_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.MaxAnisotropy = 4;
            sampler_.BorderColor[3] = 0.0f; // no alpha
            sampler_.ComparisonFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
            return sampler_;
        }

    public:
            
        void construct_vertices() const;
    
        rect_draw() = default;
        rect_draw(rect_draw&&) = default;
        rect_draw& operator=(rect_draw&&) = default;
    
        rect_draw(gtl::d3d::device& dev, gtl::d3d::command_queue& cqueue, 
                   gtl::d3d::root_signature& rsig) 
        :   layout_(vertex_layout()),            
            vbuffer_descriptors_{dev,3,gtl::d3d::tags::shader_visible{}},
            vbuffers_{{{dev,vbuffer_descriptors_.get_handle(0), MAX_RECTS  * sizeof(Vertex)},
                       {dev,vbuffer_descriptors_.get_handle(1), MAX_RECTS  * sizeof(Vertex)},
                       {dev,vbuffer_descriptors_.get_handle(2), MAX_RECTS  * sizeof(Vertex)}}},
            texture_descriptor_heap_{dev,1,gtl::d3d::tags::shader_visible{}},            
            texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue,                 
                L"D:\\images\\palettes\\greenish_palette.dds"
                },                        
            vshader_{L"rect_draw_gui_vs.cso"},
            pshader_{L"rect_draw_gui_ps.cso"},
            pso_{dev,pso_desc(dev,rsig,vshader_,pshader_)},
            sampler_heap_{dev,1},            
            sampler_{dev,sampler_desc(),sampler_heap_->GetCPUDescriptorHandleForHeapStart()}            
        {                        
            construct_vertices();            
            update_vertex_buffer(0);
            update_vertex_buffer(1);
            update_vertex_buffer(2);
        }
    
        void update_vertex_buffer(unsigned idx) const
        {
            vbuffers_[idx].update(reinterpret_cast<char*>(mesh_.data()),mesh_.size() * sizeof(Vertex));
        }

        void operator()(unsigned idx, float f, gtl::d3d::graphics_command_list& cl, 
                        gtl::d3d::D3D12Viewport const& viewport,
                        gtl::d3d::D3D12ScissorRect const& scissor,                        
                        D3D12_CPU_DESCRIPTOR_HANDLE const& rtv_handle) const
        {                                
            //update_vertex_buffer(idx);
            
            cl->SetPipelineState(pso_.get());
            auto heaps = { sampler_heap_.get(), texture_descriptor_heap_.get() };
        	cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());               
            //cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);            
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);            
            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());                                                                      
                        
            cl->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport), 0);  
            //cl->SetGraphicsRoot32BitConstants(3, 1, std::addressof(font_scale), 4);  

            D3D12_VERTEX_BUFFER_VIEW cbv_{vbuffers_[idx].resource()->GetGPUVirtualAddress(),static_cast<unsigned>(mesh_.size() * sizeof(Vertex)),sizeof(Vertex)};
            cl->IASetVertexBuffers(0, 1, &cbv_);              
            
            auto viewports = { std::addressof(viewport) };
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
            
            //float blendvalues[]{f,f,f,f};
            //cl->OMSetBlendFactor(blendvalues);

            cl->RSSetScissorRects(1,&scissor);
            cl->OMSetRenderTargets(1, &rtv_handle, TRUE, nullptr);            
            if (mesh_.size() > 1) {
                cl->DrawInstanced(static_cast<unsigned>(mesh_.size()-1),1,1,0);
            }
            // start vertex set to 1 to ignore first degenerate strip..
            
        }
    };
    
}} // namespaces
#endif
