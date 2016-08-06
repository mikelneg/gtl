#ifndef HBWWABVWAFSF_GTL_D3D_IMGUI_ADAPTER_H_
#define HBWWABVWAFSF_GTL_D3D_IMGUI_ADAPTER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::d3d::imgui_adapter
-----------------------------------------------------------------------------*/

#include <array>

#include <gtl/d3d_types.h>
#include <gtl/camera.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <imgui.h>


namespace gtl {
namespace d3d {
    
    class imgui_adapter {
        // texture
        // vertices    
        
        static constexpr unsigned MAX_VERTS = 500;
        static constexpr unsigned MAX_INDICES = 500;

        //struct vertex { float x,y; float u,v; uint32_t color; };
        using vertex_type = ImDrawVert;
        using index_type = ImDrawIdx; 


        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;   

        gtl::d3d::resource_descriptor_heap cbheap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable cbuffer_;       
        
            
        gtl::d3d::resource_descriptor_heap vert_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable vert_buffers_;
        
        d3d::resource_descriptor_heap idx_descriptor_heap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable idx_buffers_;

        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;
        gtl::d3d::srv font_texture_;
        

        gtl::d3d::vertex_shader vshader_;        
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::pipeline_state_object pso_;    
                
        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;        

        unsigned mutable idx_count{},vtx_count{};

        auto vertex_layout() {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
//{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                //{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,   (size_t)(&((ImDrawVert*)0)->pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,   (size_t)(&((ImDrawVert*)0)->uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
         //       {"VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
         //       {"VERTEX_NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
         //       {"VERTEX_BONE_IDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
         //       {"VERTEX_BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
         //
         //       {"INSTANCE_INFO", 0, DXGI_FORMAT_R16G16B16A16_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
         //       {"BONE_ARRAY_OFFSET", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
         //       {"BONE_COUNT", 0, DXGI_FORMAT_R32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
         //       {"OBJECT_ID", 0, DXGI_FORMAT_R32G32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}
         //  //   {"OBJECT_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
         //  //   {"MESH_ID", 0, DXGI_FORMAT_R32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
         //  //   {"COLOR_ANGLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
            };                         
        }

        
        
        auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig, 
            gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
            desc_.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };		    
		    desc_.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		    desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		                            
            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; 
            desc_.RasterizerState.FrontCounterClockwise = true;
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
            
            desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc_.DepthStencilState.DepthEnable = FALSE;
		    desc_.DepthStencilState.StencilEnable = FALSE;

		    desc_.SampleMask = UINT_MAX;                        
		    desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		    
            desc_.NumRenderTargets = 2;
		    desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;            
            desc_.RTVFormats[1] = DXGI_FORMAT_R32_UINT;
		    desc_.SampleDesc.Count = 1;              
            return desc_;		    
        }

        auto sampler_desc() {
            D3D12_SAMPLER_DESC sampler_{};
            sampler_.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;            
            sampler_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler_.MaxAnisotropy = 1;
            sampler_.BorderColor[3] = 0.0f; // no alpha
            sampler_.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;            
            return sampler_;
        }



    public:      
        
        imgui_adapter(gtl::d3d::device& dev, gtl::d3d::command_queue& cqueue, 
                      gtl::d3d::root_signature& rsig)
            :   
            cbheap_{dev,3,gtl::d3d::tags::shader_visible{}},
            cbuffer_{{{dev,cbheap_.get_handle(0),sizeof(gtl::camera)},{dev,cbheap_.get_handle(1),sizeof(gtl::camera)},{dev,cbheap_.get_handle(2),sizeof(gtl::camera)}}},                        
           
            layout_{vertex_layout()},
            vert_descriptor_heap_{dev,3,gtl::d3d::tags::shader_visible{}},
            vert_buffers_{{{dev,vert_descriptor_heap_.get_handle(0),MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{}},
                           {dev,vert_descriptor_heap_.get_handle(1),MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{}},
                           {dev,vert_descriptor_heap_.get_handle(2),MAX_VERTS * sizeof(ImDrawVert), gtl::d3d::tags::shader_view{}}}},
            idx_descriptor_heap_{dev,3,gtl::d3d::tags::shader_visible{}},
            idx_buffers_{{{dev,idx_descriptor_heap_.get_handle(0),MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{}},
                          {dev,idx_descriptor_heap_.get_handle(1),MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{}},
                          {dev,idx_descriptor_heap_.get_handle(2),MAX_INDICES * sizeof(ImDrawIdx), gtl::d3d::tags::shader_view{}}}},
            texture_descriptor_heap_{dev,1,gtl::d3d::tags::shader_visible{}},
            //font_texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue,                 
            //    L"D:\\images\\brutal.dds"
            //    }, 
            font_texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue, 
                            [](){                                  
                                ImGuiIO& io = ImGui::GetIO();
                                io.DisplaySize = ImVec2(960.0f,540.0f);
                                io.RenderDrawListsFn = NULL;
                                io.Fonts->TexID = 0;                                
            
                                unsigned char* pixels{};
                                int width{},height{},bytes_per_pixel{};
                                //io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height,&bytes_per_pixel);
                                io.Fonts->GetTexDataAsAlpha8(&pixels,&width,&height,&bytes_per_pixel);
                                std::vector<uint32_t> font_data; 
                                //width = 500;
                                //height = 500;
                                
                                font_data.reserve(width * height);
                                for (int m = 0; m < (height); ++m) {
                                    for (int n = 0; n < (width); ++n) {                                            
                                        font_data.emplace_back(pixels[m * width + n]);                                        
                                    }
                                }
                                auto p = font_data.data();
                                std::cout << "loaded..\n";
                                return std::make_tuple(std::move(font_data),static_cast<unsigned>(width),static_cast<unsigned>(height));
            }()},
            vshader_{L"imgui_vs.cso"},            
            pshader_{L"imgui_ps.cso"},                        
            pso_{dev,pso_desc(dev,rsig, vshader_, pshader_)},
            sampler_heap_{dev,1},            
            sampler_{dev,sampler_desc(),sampler_heap_->GetCPUDescriptorHandleForHeapStart()}            
        {           
            ImGui::NewFrame();
            ImGui::Begin("Window Title Here");
            ImGui::Text("Hello, world!");
            ImGui::End();
            ImGui::Render();            
            update(0,ImGui::GetDrawData());             
            update(1,ImGui::GetDrawData());             
            update(2,ImGui::GetDrawData());             

        }
            
        

        void update(unsigned idx, ImDrawData* draw_data) const 
        {
            // HACK no bounds checking currently..
            idx_count = 0;      
            vtx_count = 0;
            
            for (int n = 0, voffs = 0, ioffs = 0; n < draw_data->CmdListsCount; n++) {
                auto* cmd_list = draw_data->CmdLists[n];
                vert_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->VtxBuffer[0]),
                                          cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), voffs * sizeof(ImDrawVert));
                idx_buffers_[idx].update(reinterpret_cast<char*>(&cmd_list->IdxBuffer[0]),
                                         cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), ioffs * sizeof(ImDrawIdx));
                voffs += cmd_list->VtxBuffer.size();
                ioffs += cmd_list->IdxBuffer.size();

                idx_count += cmd_list->IdxBuffer.size();
                vtx_count += cmd_list->VtxBuffer.size();
            }
        }


        template <typename Camera> 
        void operator()(unsigned idx, float, gtl::d3d::graphics_command_list& cl,
                        gtl::d3d::raw::Viewport const& viewport,
                        gtl::d3d::raw::ScissorRect const& scissor,                        
                        Camera const&, // unused
                        D3D12_CPU_DESCRIPTOR_HANDLE *rtv_handle,
                        D3D12_CPU_DESCRIPTOR_HANDLE const* dbv_handle) const 
        {         
            
            ImGui::NewFrame();
            ImGui::Begin("Window Title Here");
            ImGui::Text("Hello, world!");
            ImGui::End();
            ImGui::Render();            
            update(idx,ImGui::GetDrawData());             
            

            cl->SetPipelineState(pso_.get());
            auto heaps = { sampler_heap_.get(), texture_descriptor_heap_.get() };
        	cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());               
            //cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);            
            //cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);            
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);            
            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());                                                                                              
            cl->SetGraphicsRoot32BitConstants(3, 4, std::addressof(viewport), 0);                                                                     
            //
            //cl->SetGraphicsRootShaderResourceView(4,(vert_buffers_[idx].resource())->GetGPUVirtualAddress());
            //cl->SetGraphicsRootShaderResourceView(6,(idx_buffers_[idx].resource())->GetGPUVirtualAddress());
                   
            auto viewports = { std::addressof(viewport) };
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
            
            //float blendvalues[]{f,f,f,f};
            //cl->OMSetBlendFactor(blendvalues);

            cl->RSSetScissorRects(1,&scissor);
            cl->OMSetRenderTargets(1, rtv_handle, false, dbv_handle);            
                        
            
            
            D3D12_VERTEX_BUFFER_VIEW v{vert_buffers_[idx].resource()->GetGPUVirtualAddress(),
                                       vtx_count * sizeof(vertex_type),
                                       sizeof(vertex_type)};
                    
            D3D12_INDEX_BUFFER_VIEW ibv{idx_buffers_[idx].resource()->GetGPUVirtualAddress(),
                                        idx_count * sizeof(index_type),
                                        DXGI_FORMAT_R16_UINT};//DXGI_FORMAT_R32_UINT};

            cl->IASetVertexBuffers(0, 1, &v); 
            cl->IASetIndexBuffer(&ibv); 

            //cl->DrawInstanced(6,1,0,0);            
            cl->DrawIndexedInstanced(idx_count,1,0,0,0);
        }
       

    };    

}} // namespaces
#endif


