#ifndef UWIOTOWTWAASFSAXX_GTL_OBJECT_RENDERING_SCENE_H_
#define UWIOTOWTWAASFSAXX_GTL_OBJECT_RENDERING_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::d3d
    
    class object_rendering_scene
-----------------------------------------------------------------------------*/

#include <vector>

#include <gtl/d3d_types.h>

#include <gtl/physics_simulation.h>
#include <gtl/camera.h>

#include <Eigen/Core>

#include <fstream>
#include <string>


namespace gtl {
namespace d3d {

    class object_rendering_scene {        

        static constexpr unsigned MAX_ENTITIES = 400;
        using vertex_type = Eigen::Vector4f;

        template <typename T>
        using aligned_vector = std::vector<T, Eigen::aligned_allocator<T>>;        

        std::vector<D3D12_INPUT_ELEMENT_DESC> layout_;                
        
        aligned_vector<vertex_type> mesh_;   
        gtl::d3d::vertex_buffer vbuffer_;

        gtl::d3d::resource_descriptor_heap ibuffer_descriptors_;        
        aligned_vector<EntityInfo> instance_data_;                
        std::array<gtl::d3d::constant_buffer,3> mutable ibuffers_;        
        
        std::array<gtl::d3d::resource_descriptor_heap,3> cbheap_;
        std::array<gtl::d3d::constant_buffer, 3> mutable cbuffer_;       
        
        gtl::d3d::resource_descriptor_heap texture_descriptor_heap_;        
        gtl::d3d::srv texture_;               
    
        gtl::d3d::vertex_shader vshader_;        
        gtl::d3d::pixel_shader pshader_;

        gtl::d3d::pipeline_state_object pso_;    
                
        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;        
        
        gtl::physics_simulation& physics_;
        std::vector<EntityInfo> mutable positions_;
        std::array<bool,3> mutable position_flags_;

        auto vertex_layout() {
            return std::vector<D3D12_INPUT_ELEMENT_DESC>{
                {"VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"OBJECT_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                {"COLOR_ANGLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                {"OBJECT_ID", 0, DXGI_FORMAT_R32_UINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}
            };                         
        }
  
        auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig, 
            gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
            desc_.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };		    
		    desc_.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		    desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		                            
            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;  
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
            sampler_.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;            
            sampler_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler_.MaxAnisotropy = 4;
            sampler_.BorderColor[3] = 0.0f; // no alpha
            sampler_.ComparisonFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;            
            return sampler_;
        }

        auto dummy_mesh() {
            aligned_vector<vertex_type> vector_;

            vector_.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back( 1.0f, 1.0f, 1.0f, 1.0f);
            vector_.emplace_back(-1.0f,-1.0f, 1.0f, 1.0f);
            vector_.emplace_back( 1.0f,-1.0f, 1.0f, 1.0f);
                       
            return vector_;
        }

        auto raw_mesh(std::wstring filename) {
            aligned_vector<vertex_type> vector_;
            std::ifstream file_{filename};
            std::string line_;
            while (std::getline(file_,line_)) {            
                std::istringstream input_{line_};
                float x,y,z;
                while (input_ >> x >> y >> z) {
                    vector_.emplace_back(x,y,z,1.0f);
                }                                    
            }

            return vector_;
        }

    public:
                    
    
        object_rendering_scene() = default;
        object_rendering_scene(object_rendering_scene&&) = default;
        object_rendering_scene& operator=(object_rendering_scene&&) = default;
    
        object_rendering_scene(gtl::d3d::device& dev, gtl::d3d::command_queue& cqueue, 
                   gtl::d3d::root_signature& rsig, gtl::physics_simulation& physics_) 
        :   layout_(vertex_layout()),            
            mesh_(raw_mesh(L"D:\\Meshes\\Monkey.raw")),
            vbuffer_{dev,cqueue,mesh_.data(),mesh_.size() * sizeof(vertex_type)},            
            ibuffer_descriptors_{dev,3,gtl::d3d::tags::shader_visible{}},
            ibuffers_{{{dev,ibuffer_descriptors_.get_handle(0), MAX_ENTITIES  * sizeof(EntityInfo)},
                       {dev,ibuffer_descriptors_.get_handle(1), MAX_ENTITIES  * sizeof(EntityInfo)},
                       {dev,ibuffer_descriptors_.get_handle(2), MAX_ENTITIES  * sizeof(EntityInfo)}}},
            
            cbheap_{{{dev,1,gtl::d3d::tags::shader_visible{}},{dev,1,gtl::d3d::tags::shader_visible{}},{dev,1,gtl::d3d::tags::shader_visible{}}}},
            cbuffer_{{{dev,cbheap_[0],sizeof(gtl::camera)},{dev,cbheap_[1],sizeof(gtl::camera)},{dev,cbheap_[2],sizeof(gtl::camera)}}},                        
            texture_descriptor_heap_{dev,1,gtl::d3d::tags::shader_visible{}},            
            texture_{dev, {texture_descriptor_heap_.get_handle(0)}, cqueue,                 
                L"D:\\images\\palettes\\greenish_palette.dds"
                },                        
            //vshader_{L"object_rendering_scene_gui_vs.cso"},
            //pshader_{L"object_rendering_scene_gui_ps.cso"},
            vshader_{L"object_rendering_scene_vs.cso"},            
            pshader_{L"object_rendering_scene_ps.cso"},
            //pso_{dev,pso_desc(dev,rsig,vshader_,pshader_)},
            pso_{dev,pso_desc(dev,rsig, vshader_, pshader_)},
            sampler_heap_{dev,1},            
            sampler_{dev,sampler_desc(),sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
            physics_{physics_},
            positions_{}//physics_.copy_out()}
        {                                    
            
            //positions_.clear();

            //for (size_t i = 0; i < 20; ++i) {    
            //    positions_.emplace_back(Eigen::Vector4f{vn::math::rand_neg_one_one(),vn::math::rand_neg_one_one(),
            //                            0.1f,0.1f});
            //}

            //construct_vertices(positions_);            
            //vbuffers_[0].update(reinterpret_cast<char*>(mesh_.data()),mesh_.size() * sizeof(EntityInfo));
            //vbuffers_[1].update(reinterpret_cast<char*>(mesh_.data()),mesh_.size() * sizeof(EntityInfo));
            //vbuffers_[2].update(reinterpret_cast<char*>(mesh_.data()),mesh_.size() * sizeof(EntityInfo));

            ibuffers_[0].update(reinterpret_cast<char*>(positions_.data()),positions_.size() * sizeof(EntityInfo));
            ibuffers_[1].update(reinterpret_cast<char*>(positions_.data()),positions_.size() * sizeof(EntityInfo));
            ibuffers_[2].update(reinterpret_cast<char*>(positions_.data()),positions_.size() * sizeof(EntityInfo));

            position_flags_[0] = false;         // dirty flags.. fix these..
            position_flags_[1] = false;
            position_flags_[2] = false;
        }
    
        void update_instance_buffer(unsigned idx) const
        {
            if (physics_.extract_positions(positions_)) {
                for (auto&& e : position_flags_) e = true; // set dirty
                position_flags_[idx] = false;
                //construct_vertices(positions_);
                ibuffers_[idx].update(reinterpret_cast<char*>(positions_.data()),positions_.size() * sizeof(EntityInfo));
            } else {
                if (position_flags_[idx]) {
                    position_flags_[idx] = false;
                    //construct_vertices(positions_);
                    ibuffers_[idx].update(reinterpret_cast<char*>(positions_.data()),positions_.size() * sizeof(EntityInfo));
                }
            }
        }

        void operator()(unsigned idx, float, gtl::d3d::graphics_command_list& cl,
                        gtl::d3d::raw::Viewport const& viewport,
                        gtl::d3d::raw::ScissorRect const& scissor,                        
                        Eigen::Matrix4f const& camera,
                        D3D12_CPU_DESCRIPTOR_HANDLE *rtv_handle) const
        {                                
            //update_camera_buffers(camera);
            cbuffer_[idx].update(reinterpret_cast<const char*>(&camera),sizeof(gtl::camera));  
            update_instance_buffer(idx);            
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
            
            D3D12_VERTEX_BUFFER_VIEW iaviews_[] = {
                    {vbuffer_->GetGPUVirtualAddress(), static_cast<unsigned>(mesh_.size() * sizeof(vertex_type)), sizeof(vertex_type)},
                    {ibuffers_[idx].resource()->GetGPUVirtualAddress(), static_cast<unsigned>(positions_.size() * sizeof(EntityInfo)),sizeof(EntityInfo)}};                        
            
            cl->IASetVertexBuffers(0, 2, iaviews_);                          
            
            auto viewports = { std::addressof(viewport) };
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
            
            //float blendvalues[]{f,f,f,f};
            //cl->OMSetBlendFactor(blendvalues);

            cl->RSSetScissorRects(1,&scissor);
            cl->OMSetRenderTargets(2, rtv_handle, false, nullptr);            
    
            cl->DrawInstanced(static_cast<unsigned>(mesh_.size()), static_cast<unsigned>(positions_.size()), 0, 0);
        }
    };
    
}} // namespaces
#endif
