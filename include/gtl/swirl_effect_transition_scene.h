#ifndef UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_
#define UTWOWOPQRRR_GTL_SCENES_SWIRL_EFFECT_TRANSITION_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::scenes::transitions::
    
    class swirl_effect;
-----------------------------------------------------------------------------*/

#include <gtl/events.h>
#include <gtl/keyboard_enum.h>

#include <Windows.h>
#include <windowsx.h>

#include <cstddef>
#include <array>
#include <utility>

#include <gtl/gtl_window.h>
#include <gtl/d3d_types.h>
#include <gtl/d3d_funcs.h>
#include <gtl/copyable_atomic.h>

#include <gtl/gui_rect_draw.h>
#include <gtl/object_rendering_scene.h>
#include <gtl/d3d_imgui_adapter.h>

#include <vn/math_utilities.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <vector>
#include <cmath>
#include <random>
#include <atomic>

#include <gtl/stage.h>
#include <gtl/command_variant.h>

#include <gtl/physics_simulation.h>

namespace gtl {
namespace scenes {
namespace transitions {

 
inline float const& pi() 
{   
    static float const value{static_cast<float>(std::atan2(0, -1))};
    return value; 
}

inline float to_radians(float degrees) 
{ 
    return (degrees * (pi() / 180.f));  
}

inline float to_degrees(float radians) 
{ 
    return (radians * (180.0f / pi())); 
}

inline Eigen::Matrix4f makeProjectionMatrix(float fov_y, float aspect_ratio, float z_near, float z_far)
{
    
    float s = 1.0f / std::tan(fov_y * 0.5f);
    Eigen::Matrix4f matrix_;

    //  f(z,near,far) = [ z * (1/(far-near)) + (-near/(far-near)) ] / z
    //  maps z between near and far to [0...1]
    //  recommended that you graph this with your near/far values to examine
    //  the scale

    matrix_ << s/aspect_ratio, 0.0f, 0.0f, 0.0f,
               0.0f, s, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f/(z_far-z_near), 1.0f,
               0.0f, 0.0f, -z_near/(z_far-z_near), 0.0f;
    return matrix_;
}

    template <typename T> // TODO garbage, change this..
    void update(T& cb) {
        using namespace Eigen;
        static Quaternionf orientation_{Quaternionf::Identity().normalized()};
        static Quaternionf rot_{Quaternionf::FromTwoVectors(Vector3f{0.0f,0.0f,1.0f},
                                                            Vector3f{0.0001f,0.0001f,1.0f}.normalized()
                                                           ).normalized()};     
        
        static auto const proj_mat = makeProjectionMatrix(to_radians(30.0f), 960/540.0f, 0.0001f, 1.0f);
    
        orientation_ = orientation_ * rot_;
        Affine3f transform_{Affine3f::Identity()};
        transform_.rotate(orientation_.toRotationMatrix());
    
        cb.view_matrix = transform_.matrix() * proj_mat;
        //cb.view_matrix = Eigen::Matrix4f{};
    };

    struct cbuffer {
        Eigen::Matrix4f view_matrix{};        

        cbuffer() {
            update(*this);
        }

    };       
    

    class swirl_effect {        
        
        constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

        //gtl::stage& stage_;

        gtl::d3d::device& dev_;
        gtl::d3d::command_queue& cqueue_;        
        gtl::d3d::swap_chain& swchain_;

        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;
        gtl::d3d::root_signature root_sig_;
       
        //std::array<gtl::d3d::resource_descriptor_heap,frame_count> cbheap_;
        gtl::d3d::resource_descriptor_heap cbheap_;

        cbuffer mutable cbuf_;                            
        std::array<gtl::d3d::constant_buffer, frame_count> mutable cbuffer_;       
        gtl::d3d::depth_stencil_buffer depth_buffers_;        

        gtl::d3d::pipeline_state_object pso_;

        std::array<gtl::d3d::direct_command_allocator,frame_count> calloc_;
        std::array<gtl::d3d::graphics_command_list,frame_count> clist_;
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable gui_rect_clist_;
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable object_effect_clist_;
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable imgui_clist_;
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable id_sampler_clist_;

        //std::array<std::atomic<int32_t>, frame_count> mutable ids_;
        
        gtl::d3d::raw::Viewport viewport_;//{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
        gtl::d3d::raw::ScissorRect scissor_;//{0,0,960,540};    

        gtl::d3d::resource_descriptor_heap resource_heap_;
        gtl::d3d::srv texture_;     
        gtl::d3d::rtv_srv_texture2D mutable id_layer_;
        gtl::d3d::resource mutable id_readback_;        

        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        gtl::d3d::sampler sampler_;      

        gtl::d3d::rect_draw gui_rects_;
        gtl::d3d::object_rendering_scene object_effect_;
        gtl::d3d::imgui_adapter imgui_;

        gtl::copyable_atomic<int64_t> mutable mouse_coord_;        
        
        auto pso_desc(gtl::d3d::device&, gtl::d3d::root_signature& rsig, gtl::d3d::vertex_shader& vs, gtl::d3d::pixel_shader& ps) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
            desc_.pRootSignature = rsig.get();
		    desc_.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
		    desc_.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };        
		    desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		                            

            //typedef struct D3D12_INPUT_ELEMENT_DESC
            //{
            //LPCSTR SemanticName;
            //UINT SemanticIndex;
            //DXGI_FORMAT Format;
            //UINT InputSlot;
            //UINT AlignedByteOffset;
            //D3D12_INPUT_CLASSIFICATION InputSlotClass;
            //UINT InstanceDataStepRate;

            //D3D11_RASTERIZER_DESC desc_{};
            //desc_.CullMode = D3D11_CULL_NONE; // BACK
            //desc_.FillMode = D3D11_FILL_SOLID;
            //desc_.FrontCounterClockwise = true;
            //desc_.DepthBias = 0;
            //desc_.DepthBiasClamp = 0;
            //desc_.SlopeScaledDepthBias = 0;
            //desc_.DepthClipEnable = false;
            //desc_.ScissorEnable = false;
            //desc_.MultisampleEnable = true;
            //desc_.AntialiasedLineEnable = true;
            desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;  
            desc_.RasterizerState.FrontCounterClockwise = false;
            desc_.RasterizerState.DepthClipEnable = true;
            desc_.RasterizerState.AntialiasedLineEnable = true;            
            
            D3D12_BLEND_DESC blend_desc_ = {};//CD3DX12_BLEND_DESC(D3D12_DEFAULT);                  
                blend_desc_.RenderTarget[0].BlendEnable = true;
                blend_desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_BLEND_FACTOR;
                blend_desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_BLEND_FACTOR;                
                blend_desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
                blend_desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;                 
                blend_desc_.RenderTarget[0].BlendEnable = true;
       
            desc_.BlendState = blend_desc_;                                                         

            desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            //desc_.DepthStencilState.FrontFace.StencilDepthFailOp;
            desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            desc_.DepthStencilState.DepthEnable = TRUE;
		    desc_.DepthStencilState.StencilEnable = FALSE;

		    desc_.SampleMask = UINT_MAX;                        
		    desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;            
		    //desc_.NumRenderTargets = 1; // swap chain & id system..
            desc_.NumRenderTargets = 2;
		    desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc_.RTVFormats[1] = DXGI_FORMAT_R32_UINT;
		    desc_.SampleDesc.Count = 1;              
            return desc_;		    
        }


    public:
        swirl_effect(gtl::d3d::device& dev_, gtl::d3d::swap_chain& swchain_, gtl::d3d::command_queue& cqueue_, gtl::physics_simulation& physics_)
                  //   gtl::stage& stage_) // TODO temporary effect..
            :  // stage_{stage_},

                dev_{dev_}, 
                cqueue_{cqueue_},                
                swchain_{swchain_},
                vshader_{L"D:\\Code\\D3D12_migration\\D3D12_migration\\Debug\\x64\\skybox_vs.cso"},
                pshader_{L"D:\\Code\\D3D12_migration\\D3D12_migration\\Debug\\x64\\skybox_ps.cso"},
                root_sig_{dev_, vshader_},
                //cbheap_{{{dev_,1,gtl::d3d::tags::shader_visible{}},{dev_,1,gtl::d3d::tags::shader_visible{}},{dev_,1,gtl::d3d::tags::shader_visible{}}}},                                                     
                cbheap_{dev_,frame_count,gtl::d3d::tags::shader_visible{}},
                cbuf_{},                
                cbuffer_{{{dev_,cbheap_.get_handle(0),sizeof(cbuf_)},{dev_,cbheap_.get_handle(1),sizeof(cbuf_)},{dev_,cbheap_.get_handle(2),sizeof(cbuf_)}}},                                        
                depth_buffers_{swchain_},    
                pso_{dev_, pso_desc(dev_, root_sig_, vshader_, pshader_)},
                calloc_{{{dev_},{dev_},{dev_}}},
                clist_{{{dev_,calloc_[0],pso_},{dev_,calloc_[1],pso_},{dev_,calloc_[2],pso_}}},
                gui_rect_clist_{{{dev_,calloc_[0]},{dev_,calloc_[1]},{dev_,calloc_[2]}}},
                object_effect_clist_{{{dev_,calloc_[0]},{dev_,calloc_[1]},{dev_,calloc_[2]}}},
                imgui_clist_{{{dev_,calloc_[0]},{dev_,calloc_[1]},{dev_,calloc_[2]}}},
                id_sampler_clist_{{{dev_,calloc_[0]},{dev_,calloc_[1]},{dev_,calloc_[2]}}},
                viewport_{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f},
                scissor_{0,0,960,540},
                resource_heap_{dev_,2,gtl::d3d::tags::shader_visible{}},
                texture_{dev_,{resource_heap_->GetCPUDescriptorHandleForHeapStart()},cqueue_,L"D:\\images\\skyboxes\\Nightsky.dds"},
                id_layer_{swchain_, DXGI_FORMAT_R32_UINT, 3, gtl::d3d::tags::shader_visible{}},                
                sampler_heap_{dev_,1},
                sampler_{dev_,sampler_heap_->GetCPUDescriptorHandleForHeapStart()},
                gui_rects_{dev_, cqueue_, root_sig_, physics_},
                object_effect_{dev_, cqueue_, root_sig_, physics_},
                imgui_{dev_, cqueue_, root_sig_}
        {            
            //
            dev_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),    // TODO add d3d readback type
                                          D3D12_HEAP_FLAG_NONE,
                                          &CD3DX12_RESOURCE_DESC::Buffer(256),
                                          D3D12_RESOURCE_STATE_COPY_DEST,
                                          nullptr, __uuidof(gtl::d3d::resource::type),
                                          void_ptr(id_readback_));            
            // cbuffer_[idx].update() -- 
            std::cout << "swirl_effect()\n";
        }

        void set_mouse_coords(int64_t i) const { mouse_coord_.set(i); }

        swirl_effect& operator=(swirl_effect&&) { std::cout << "swirl_effect operator= called..\n"; return *this; } // TODO throw? assert false?
        swirl_effect(swirl_effect&&) = default;

        ~swirl_effect() { std::cout << "~swirl_effect()\n"; }

        //template <typename T>
        //void operator()(T const&) const { std::cout << "swirl_effect caught generic message, not handling..\n"; }

        //void operator()(gtl::commands::draw const&) const {            
        //    stage_.draw_callback([this](auto&&...ps){ draw(std::forward<decltype(ps)>(ps)...); });
        //}


//        std::vector<ID3D12CommandList*> draw(int idx, float f, gtl::d3d::rtv_descriptor_heap& rtv_heap_) const {            
//            update(cbuf_);
//            cbuffer_[idx].update(reinterpret_cast<const char*>(&cbuf_),sizeof(cbuf_));  
//
//            // copy id layer into id_readback
//            
//            uint32_t id_value_{};
//                        
//            //id_readback_->ReadFromSubresource(&id_value_, 0, 0, 0, &CD3DX12_BOX{0,1});
//            void *p{};            
//            id_readback_->Map(0, &CD3DX12_RANGE{0,4},&p);
//            memcpy(&id_value_, p, 4);
//            id_readback_->Unmap(0, &CD3DX12_RANGE{1,0}); // end < begin tells the api that nothing was written
//            
//
//
////            
////
////            void CopyBufferRegion(
////                  [in] ID3D12Resource *pDstBuffer,
////                       UINT64         DstOffset,
////                  [in] ID3D12Resource *pSrcBuffer,
////                       UINT64         SrcOffset,
////                       UINT64         NumBytes
////                );
////
////            
////
////            void CopyTextureRegion(
////  [in]           const D3D12_TEXTURE_COPY_LOCATION *pDst,
////                       UINT                        DstX,
////                       UINT                        DstY,
////                       UINT                        DstZ,
////  [in]           const D3D12_TEXTURE_COPY_LOCATION *pSrc,
////  [in, optional] const D3D12_BOX                   *pSrcBox
////);
////                
//            //            
//            std::vector<ID3D12CommandList*> v;
//            calloc_[idx]->Reset();
//            clist_[idx]->Reset(calloc_[idx].get(),pso_.get());
//            //
//            gtl::d3d::graphics_command_list const& cl = clist_[idx];            
//            
//            cl->SetGraphicsRootSignature(root_sig_.get());
//            auto heaps = { sampler_heap_.get(), resource_heap_.get() };
//        	cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());                    
//            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
//            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
//            cl->SetGraphicsRootDescriptorTable(2, resource_heap_->GetGPUDescriptorHandleForHeapStart());
//                                                                    
//            float blendvalues[]{f,f,f,f};
//            cl->OMSetBlendFactor(blendvalues);
//
//            auto viewports = { std::addressof(viewport_) };
//            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
//            cl->RSSetScissorRects(1, std::addressof(scissor_));            
//            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//            CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};                                                    
//            rtv_handle.Offset(idx, rtv_heap_.increment_value());
//
//            CD3DX12_CPU_DESCRIPTOR_HANDLE id_handle{id_layer_.rtv_heap()->GetCPUDescriptorHandleForHeapStart()};                                                    
//            id_handle.Offset(idx, id_layer_.rtv_heap().increment_value());
//
//            D3D12_CPU_DESCRIPTOR_HANDLE handles[]{rtv_handle,id_handle};
//
//            //cl->OMSetRenderTargets(2, handles, false, nullptr); // not issuing ids with this shader..
//            cl->OMSetRenderTargets(1, &rtv_handle, TRUE, nullptr);
//            cl->DrawInstanced(14, 1, 0, 0);             
//            clist_[idx]->Close();            
//            
//            gui_rect_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
//            gui_rect_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           
//            
//            gui_rects_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,handles);
//                            
//            D3D12_TEXTURE_COPY_LOCATION src{id_layer_, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
//            D3D12_TEXTURE_COPY_LOCATION dst{id_readback_, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 
//                                                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0,
//                                CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT,1,1,1,256}}};
//
//            src.SubresourceIndex = idx;            
//            //dst.PlacedFootprint = D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0,CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT,4,1,0,16}};
//
//            // id layer texture copy..
//            gui_rect_clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                id_layer_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, idx));
//
//            //gui_rect_clist_[idx]->CopyBufferRegion(id_readback_,0,id_layer_,0,4);
//
//            LPARAM coord_ = mouse_coord_.get();
//            int mx = GET_X_LPARAM(coord_);
//            int my = GET_Y_LPARAM(coord_);
//
//            static int i = 0;
//            if (i++ > 30) {
//                i = 0;
//                std::cout << "mouse @ " << mx << "," << my << " :: id == " << id_value_ << "\n"; 
//                //std::cout << "id value == " << id_value_ << "\n"; 
//            }
//
//            if (mx > 0 && 960 > mx && my > 0 && 540 > my) {
//                gui_rect_clist_[idx]->CopyTextureRegion(&dst,0,0,0,&src,
//                                        //&CD3DX12_BOX{200,200,201,201});
//                                        &CD3DX12_BOX{mx,my,mx+1,my+1});
//            }
//                                    
//            gui_rect_clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//                id_layer_, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, idx));            
//
//            gui_rect_clist_[idx]->Close();
//
//            //
//            //id_sampler_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
//            //id_sampler_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           
//            ////id_sampler_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,rtv_handle);
//            //id_sampler_clist_[idx]->Close();
//
//            //
//            v.emplace_back(clist_[idx].get());
//            v.emplace_back(gui_rect_clist_[idx].get());
//            //v.emplace_back(id_sampler_clist_[idx].get());
//            
//            return v;
//        }
        
        void draw(std::vector<ID3D12CommandList*>& v, 
                  int idx, float f, gtl::d3d::rtv_descriptor_heap& rtv_heap_, 
                  std::atomic<uint32_t>& id_,
                  Eigen::Matrix4f const& camera) const {           

            update(cbuf_);
            cbuffer_[idx].update(reinterpret_cast<const char*>(&cbuf_),sizeof(cbuf_));  

            // copy id layer into id_readback
            
            uint32_t id_value_{};
                        
            //id_readback_->ReadFromSubresource(&id_value_, 0, 0, 0, &CD3DX12_BOX{0,1});
            void *p{};            
            id_readback_->Map(0, &CD3DX12_RANGE{0,4},&p);
            memcpy(&id_value_, p, 4);
            id_readback_->Unmap(0, &CD3DX12_RANGE{1,0}); // end < begin tells the api that nothing was written
            
            id_.store(id_value_, std::memory_order_relaxed);

//            
//
//            void CopyBufferRegion(
//                  [in] ID3D12Resource *pDstBuffer,
//                       UINT64         DstOffset,
//                  [in] ID3D12Resource *pSrcBuffer,
//                       UINT64         SrcOffset,
//                       UINT64         NumBytes
//                );
//
//            
//
//            void CopyTextureRegion(
//  [in]           const D3D12_TEXTURE_COPY_LOCATION *pDst,
//                       UINT                        DstX,
//                       UINT                        DstY,
//                       UINT                        DstZ,
//  [in]           const D3D12_TEXTURE_COPY_LOCATION *pSrc,
//  [in, optional] const D3D12_BOX                   *pSrcBox
//);
//                
            //            
            //std::vector<ID3D12CommandList*> v;
            calloc_[idx]->Reset();
            clist_[idx]->Reset(calloc_[idx].get(),pso_.get());
            //
            gtl::d3d::graphics_command_list const& cl = clist_[idx];            
            
            cl->SetGraphicsRootSignature(root_sig_.get());
            auto heaps = { sampler_heap_.get(), resource_heap_.get() };
        	cl->SetDescriptorHeaps(static_cast<unsigned>(heaps.size()), heaps.begin());                    
            cl->SetGraphicsRootConstantBufferView(0, (cbuffer_[idx].resource())->GetGPUVirtualAddress());
            cl->SetGraphicsRootDescriptorTable(1, sampler_heap_->GetGPUDescriptorHandleForHeapStart());
            cl->SetGraphicsRootDescriptorTable(2, resource_heap_->GetGPUDescriptorHandleForHeapStart());
                                                                    
            float blendvalues[]{f,f,f,f};
            cl->OMSetBlendFactor(blendvalues);

            auto viewports = { std::addressof(viewport_) };
            cl->RSSetViewports(static_cast<unsigned>(viewports.size()),*viewports.begin());
            cl->RSSetScissorRects(1, std::addressof(scissor_));            
            cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{rtv_heap_->GetCPUDescriptorHandleForHeapStart()};                                                    
            rtv_handle.Offset(idx, rtv_heap_.increment_value());

            CD3DX12_CPU_DESCRIPTOR_HANDLE id_handle{id_layer_.rtv_heap()->GetCPUDescriptorHandleForHeapStart()};                                                    
            id_handle.Offset(idx, id_layer_.rtv_heap().increment_value());

            D3D12_CPU_DESCRIPTOR_HANDLE handles[]{rtv_handle,id_handle};

            //cl->OMSetRenderTargets(2, handles, false, nullptr); // not issuing ids with this shader..
            float const clearvalues[]{0.0f,0.0f,0.0f,0.0f};
            cl->ClearRenderTargetView(id_handle,clearvalues,0,nullptr);   
            
            auto const dbview_ = depth_buffers_.get_handle();
            cl->ClearDepthStencilView(dbview_,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr);            

            cl->OMSetRenderTargets(1, &rtv_handle, TRUE, std::addressof(dbview_));            
            cl->DrawInstanced(14, 1, 0, 0);             
            clist_[idx]->Close();            
            
            gui_rect_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
            gui_rect_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           
            
            // gui_rect_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,camera,handles);
            object_effect_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,camera,handles,std::addressof(dbview_));
                            
            D3D12_TEXTURE_COPY_LOCATION src{id_layer_, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
            D3D12_TEXTURE_COPY_LOCATION dst{id_readback_, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 
                                                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0,
                                CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT,1,1,1,256}}};

            src.SubresourceIndex = idx;            
            //dst.PlacedFootprint = D3D12_PLACED_SUBRESOURCE_FOOTPRINT{0,CD3DX12_SUBRESOURCE_FOOTPRINT{DXGI_FORMAT_R32_UINT,4,1,0,16}};

            // id layer texture copy..
            gui_rect_clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                id_layer_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, idx));

            //gui_rect_clist_[idx]->CopyBufferRegion(id_readback_,0,id_layer_,0,4);

            LPARAM coord_ = mouse_coord_.get();
            int mx = GET_X_LPARAM(coord_);
            int my = GET_Y_LPARAM(coord_);

            static int i = 0;
            if (++i > 50) {
                i = 0;
                std::cout << "mouse @ " << mx << "," << my << " :: id == " << id_value_ << "\n"; 
                //std::cout << "id value == " << id_value_ << "\n"; 
            }

            if (mx > 0 && 960 > mx && my > 0 && 540 > my) {
                gui_rect_clist_[idx]->CopyTextureRegion(&dst,0,0,0,&src,
                                        //&CD3DX12_BOX{200,200,201,201});
                                        &CD3DX12_BOX{mx,my,mx+1,my+1});
            }
                                    
            gui_rect_clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                id_layer_, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, idx));            

            gui_rect_clist_[idx]->Close();

            imgui_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
            imgui_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           

            imgui_(idx, f, imgui_clist_[idx],viewport_,scissor_,camera,handles,std::addressof(dbview_));

            imgui_clist_[idx]->Close();
            


            //object_effect_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
            //object_effect_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           
            //                        
            //object_effect_(idx, f, object_effect_clist_[idx],viewport_,scissor_,camera,handles);
            //object_effect_clist_[idx]->Close();
            //
            //
            //id_sampler_clist_[idx]->Reset(calloc_[idx].get(),nullptr);      
            //id_sampler_clist_[idx]->SetGraphicsRootSignature(root_sig_.get());                           
            ////id_sampler_(idx,f,gui_rect_clist_[idx],viewport_,scissor_,rtv_handle);
            //id_sampler_clist_[idx]->Close();

            //
            v.emplace_back(clist_[idx].get());
            v.emplace_back(gui_rect_clist_[idx].get());
            v.emplace_back(imgui_clist_[idx].get());
            //v.emplace_back(object_effect_clist_[idx].get());
            //v.emplace_back(id_sampler_clist_[idx].get());
            
            //return v;
        }
        

        // ^^ vv ^^ vv ^^ vv

        //template <typename YieldType>
        //gtl::event handle_events(YieldType& yield) const {            
        //    namespace ev = gtl::events;
        //    namespace k = gtl::keyboard;
        //    int count{};
        //    std::cout << "swirl_effect event handler entered..\n"; 
        //    while (!same_type(yield().get(),ev::exit_immediately{})){                   
        //        if (same_type(yield.get(),ev::keydown{})){ 
        //            
        //            switch( boost::get<ev::keydown>( yield.get().value() ).key ) {
        //                case k::Escape : std::cout << "swirl_effect(): escape pressed, exiting all..\n"; 
        //                                 return gtl::events::exit_all{}; break;
        //                case k::Q : std::cout << "swirl_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
        //                            return gtl::events::exit_state{0}; break;
        //                case k::K : std::cout << "swirl_effect(): k pressed, throwing (none == " << count << ")\n";                                                
        //                            throw std::runtime_error{__func__}; break;                    
        //                case k::R : std::cout << "swirl_effect() : r pressed, resizing swapchain..\n"; 
        //                            swchain_.resize(100,100); 
        //                            break;
        //                default : std::cout << "swirl_effect() : unknown key pressed\n"; 
        //            }
        //                           
        //        } else if (same_type(yield.get(),ev::none{})) {
        //            count++;                
        //        } else if (same_type(yield.get(),ev::mouse_at{})) {
        //            mouse_coord_.set(boost::get<ev::mouse_at>(yield.get().value()).coord);                    
        //        }
        //    }            
        //    return gtl::events::exit_state{0};
        //}            

                
        //void operator()(gtl::commands::handle const&) const {
        //    stage_.replace_event_handler([this](auto& yield){ handle_events(yield); });
        //}            
    };

}}} // namespaces
#endif


/*
#include <gtl/gtl_window.h>
#include <gtl/d3d_types.h>
#include <gtl/d3d_funcs.h>

#include <gtl/synchronized.h>

#include <gtl/clist_skybox.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>


#include <cmath>
float const& pi() 
{   
    static float const value{static_cast<float>(std::atan2(0, -1))};
    return value; 
}

float to_radians(float degrees) 
{ 
    return (degrees * (pi() / 180.f));  
}

float to_degrees(float radians) 
{ 
    return (radians * (180.0f / pi())); 
}

Eigen::Matrix4f makeProjectionMatrix(float fov_y, float aspect_ratio, float z_near, float z_far)
{
    
    float s = 1.0f / std::tan(fov_y * 0.5f);
    Eigen::Matrix4f matrix_;

    //  f(z,near,far) = [ z * (1/(far-near)) + (-near/(far-near)) ] / z
    //  maps z between near and far to [0...1]
    //  recommended that you graph this with your near/far values to examine
    //  the scale

    matrix_ << s/aspect_ratio, 0.0f, 0.0f, 0.0f,
               0.0f, s, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f/(z_far-z_near), 1.0f,
               0.0f, 0.0f, -z_near/(z_far-z_near), 0.0f;
    return matrix_;
}

struct cbuffer {
    Eigen::Matrix4f view_matrix;    
};
  
void update(cbuffer& cb) {
    using namespace Eigen;
    static Quaternionf orientation_{Quaternionf::Identity()};
    static Quaternionf rot_{Quaternionf::FromTwoVectors(Vector3f{0.0f,0.0f,1.0f},
                                                        Vector3f{0.00001f,0.000001f,1.0f}.normalized()
                                                       ).normalized()};     
    
    static auto const proj_mat = makeProjectionMatrix(to_radians(30.0f), 960/540.0f, 0.0001f, 1.0f);

    orientation_ = orientation_ * rot_;
    Affine3f transform_{Affine3f::Identity()};
    transform_.rotate(orientation_.toRotationMatrix());

    cb.view_matrix = transform_.matrix() * proj_mat;
};


int main(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{          
    try {               
        gtl::window win_{hinst, 960, 540, u8"Main Window"};                
        gtl::d3d::debug_layer debug_;        
        gtl::d3d::device dev_; 
        
        {                
        constexpr size_t frame_count = 3;

        gtl::d3d::command_queue cqueue_{dev_};
        gtl::d3d::swap_chain swchain_{win_,cqueue_,3};
        
        std::array<gtl::d3d::resource_descriptor_heap,frame_count> cbheap_{{{dev_,2,gtl::d3d::tags::shader_visible{}},
                                                                            {dev_,2,gtl::d3d::tags::shader_visible{}},
                                                                            {dev_,2,gtl::d3d::tags::shader_visible{}}}};
        gtl::d3d::sampler_descriptor_heap sampler_heap_{dev_,1};
        
        
        std::array<gtl::d3d::direct_command_allocator,frame_count> calloc_{{{dev_},{dev_},{dev_}}};             
        std::array<gtl::d3d::direct_command_allocator,frame_count> cs_alloc_{{{dev_},{dev_},{dev_}}};             

        gtl::d3d::root_signature root_sig_{dev_, gtl::d3d::dummy_rootsig_1()};
        gtl::d3d::root_signature cs_root_sig_{dev_, gtl::d3d::dummy_rootsig_2()};
        gtl::d3d::vertex_shader vshader_{L"skybox_vs.cso"};
        gtl::d3d::pixel_shader pshader_{L"skybox_ps.cso"};            
        gtl::d3d::compute_shader cshader_{L"demo_cs_cs.cso"};
        gtl::d3d::vertex_shader csvshader_{L"demo_cs_vs.cso"};
        gtl::d3d::pixel_shader cspshader_{L"demo_cs_ps.cso"};
        gtl::d3d::pipeline_state_object pso_{dev_, cs_root_sig_, csvshader_, cspshader_};
        gtl::d3d::pipeline_state_object cs_pso_{dev_, cs_root_sig_, cshader_};
        
        std::array<gtl::d3d::graphics_command_list,frame_count> clist_{{{dev_,calloc_[0],pso_},{dev_,calloc_[1],pso_},{dev_,calloc_[2],pso_}}};
        //std::array<gtl::d3d::graphics_command_list,frame_count> clist_b_{{{dev_,calloc_[0],pso_},{dev_,calloc_[1],pso_},{dev_,calloc_[2],pso_}}};
        std::array<gtl::d3d::graphics_command_list,frame_count> cs_list_{{{dev_,cs_alloc_[0],cs_pso_},{dev_,cs_alloc_[1],cs_pso_},{dev_,cs_alloc_[2],cs_pso_}}};
        
        
        gtl::d3d::D3D12Viewport viewport_{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f};
        gtl::d3d::D3D12ScissorRect scissor_{0,0,960,540};    
                      
        gtl::d3d::D3D12Viewport viewport1_{0.0f,0.0f,320.0f, 240.0f, 0.0f, 1.0f};
        gtl::d3d::D3D12Viewport viewport2_{322.0f,0.0f,960.0f,240.0f, 0.0f, 1.0f};

        cbuffer cbuf_;                    
        std::pair<char*,size_t> cbuf_data{reinterpret_cast<char*>(&cbuf_), sizeof(cbuf_)};        
        update(cbuf_);
                
        std::array<gtl::d3d::constant_buffer,frame_count> cbuffer_{{{dev_,cbheap_[0],cbuf_data},{dev_,cbheap_[0],cbuf_data},{dev_,cbheap_[0],cbuf_data}}};
                                                     
        //std::array<gtl::d3d::rtv_srv_texture2D, 3> gbuffers{{{swchain_, 2, gtl::d3d::tags::shader_visible{}},
        //                                                     {swchain_, 2, gtl::d3d::tags::shader_visible{}},
        //                                                     {swchain_, 2, gtl::d3d::tags::shader_visible{}}}};        

        std::array<gtl::d3d::CpuDescriptorHandle,frame_count> handles{{
                                   {cbheap_[0]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[0].increment_value()},
                                   {cbheap_[1]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[1].increment_value()},
                                   {cbheap_[2]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[2].increment_value()}}};        
                
        gtl::d3d::srv texture_{dev_,{handles[0],handles[1],handles[2]},cqueue_,L"D:\\images\\skyboxes\\Nightsky.dds"};

        handles[0].Offset(1, cbheap_[1].increment_value());
        handles[1].Offset(1, cbheap_[1].increment_value());
        handles[2].Offset(1, cbheap_[1].increment_value());

        std::array<gtl::d3d::uav_texture2D, 3> ubuffers{{{swchain_,handles[0]},
                                                         {swchain_,handles[1]},
                                                         {swchain_,handles[2]}}};        
       
        gtl::d3d::rtv_descriptor_heap uav_rtv{dev_,1};

        dev_->CreateRenderTargetView(ubuffers[0].get(), nullptr, uav_rtv->GetCPUDescriptorHandleForHeapStart());
        
        gtl::d3d::sampler sampler_{dev_,sampler_heap_->GetCPUDescriptorHandleForHeapStart()};

        gtl::d3d::synchronized sync_{cqueue_,gtl::d3d::fence{dev_}, 2, 2}; // framecount - 1

        auto load_text = [&](auto idx){ 
                                 std::initializer_list<D3D12_VIEWPORT*> viewports{&viewport_};                                 
                                 calloc_[idx]->Reset(); 
                                 clist_[idx]->Reset(calloc_[idx].get(), pso_.get());

                                 clist_[idx]->SetGraphicsRootSignature(cs_root_sig_.get());                                                                  
                                  
                                 ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap_.get(), cbheap_[idx].get() };
	                             clist_[idx]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        

                                 //clist_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());                                                                  
                                 clist_[idx]->SetGraphicsRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());
                                 clist_[idx]->SetGraphicsRootDescriptorTable(1, sampler_heap_.get()->GetGPUDescriptorHandleForHeapStart());
                                 clist_[idx]->RSSetViewports(1, *viewports.begin());
	                             clist_[idx]->RSSetScissorRects(1, &scissor_);    

	                             clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	                             
                                 std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE> dhandles{uav_rtv->GetCPUDescriptorHandleForHeapStart()};

                                 clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          ubuffers[idx].get(), 
                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
                                          D3D12_RESOURCE_STATE_RENDER_TARGET));

                                 //float blend[]{0.1f,0.1f,0.1f,0.1f};
                                 float blend[]{0.9f,0.0f,0.9f,0.9f};
                                 float col[]{1.0f,1.0f,1.0f,1.0f};

                                 clist_[idx]->ClearRenderTargetView(uav_rtv->GetCPUDescriptorHandleForHeapStart(),col,0,nullptr);

                                 clist_[idx]->OMSetRenderTargets(1, dhandles.begin(), TRUE, nullptr);    
                                 
                                 clist_[idx]->OMSetBlendFactor(blend);
                                 
                                 clist_[idx]->DrawInstanced(3, 1, 0, 0);                                 

                                 clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          ubuffers[idx].get(), 
                                          D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

                                 //clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                 //         ubuffers[idx].get(), 
                                 //         D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
                                 //         D3D12_RESOURCE_STATE_COPY_SOURCE));
                                 //
                                 //clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                 //         swchain_.get_current_resource(), 
                                 //         D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                 //         D3D12_RESOURCE_STATE_COPY_DEST));
                                 //
                                 //
                                 //clist_[idx]->CopyResource(swchain_.get_current_resource(), ubuffers[idx].get());
                                 //
                                 //clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                 //         swchain_.get_current_resource(), 
                                 //         D3D12_RESOURCE_STATE_COPY_DEST, 
                                 //         D3D12_RESOURCE_STATE_PRESENT));                                 
                                 //
                                 clist_[idx]->Close();

                                 std::initializer_list<ID3D12CommandList*> clists{clist_[idx].get()};                                                                          
                                 cqueue_->ExecuteCommandLists(gtl::win::array_size(clists), clists.begin()); 
                                 //swchain_->Present(0,0);
                                 };



        auto single_pass = [&](auto idx){                                  
                                 cs_alloc_[idx]->Reset(); 
                                 cs_list_[idx]->Reset(cs_alloc_[idx].get(), cs_pso_.get());

                                 cs_list_[idx]->SetComputeRootSignature(cs_root_sig_.get());                                                                  
                                  
                                 ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap_.get(), cbheap_[idx].get() };
	                             cs_list_[idx]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        

                                 //clist_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());                                                                  
                                 cs_list_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());
                                 ////clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_.get()->GetGPUDescriptorHandleForHeapStart());                                                                                                   
                                 //clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_[idx]->GetGPUDescriptorHandleForHeapStart());     
                                 
                                 cs_list_[idx]->Dispatch(120,68,1);                                    
                                 cs_list_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          ubuffers[idx].get(), 
                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE));
                                 gtl::d3d::graphics_command_lists::skybox_graphics_command_list_second(
                                     cs_list_[idx],swchain_.get_current_resource(), ubuffers[idx]);                                 

                                 std::initializer_list<ID3D12CommandList*> clists{cs_list_[idx].get()};                                         
                                 if (WaitForSingleObject(swchain_->GetFrameLatencyWaitableObject(),0) == WAIT_OBJECT_0) {                                                         	                    
                                     cqueue_->ExecuteCommandLists(gtl::win::array_size(clists), clists.begin());
                                     swchain_->Present(0,0);
                                     return true;
                                 }            
                                 return false;                 
                                 };
        load_text(0);
        wait_for_gpu(dev_,cqueue_);
        single_pass(0);

        //

        MSG msg{WM_NULL};
        while (msg.message != WM_QUIT) {
            
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }        

            auto graphics_pass = [&](auto idx){ 
                                 update(cbuf_);
                                 cbuffer_[idx].update(cbuf_data);           
                                 
                                 //std::initializer_list<D3D12_VIEWPORT*> viewports{&viewport1_, &viewport2_};
                                 std::initializer_list<D3D12_VIEWPORT*> viewports{&viewport_};

                                 //gtl::d3d::graphics_command_lists::skybox_graphics_command_list(clist_[idx],calloc_[idx],pso_,
                                 //                                             root_sig_,viewports, scissor_,
                                 //                                             swchain_.get_current_resource(), 
                                 //                                             gbuffers[idx],                                                                              
                                 //                                             //swchain_.get_handle_to_current_resource(),                                                          
                                 //                                             cbuffer_[idx].resource(),cbheap_, sampler_heap_);  

                                 calloc_[idx]->Reset();
                                 //cs_alloc_[idx]->Reset();

                                 clist_[idx]->Reset(calloc_[idx].get(), pso_.get());                                 
                                 //clist_[idx]->SetComputeRootSignature(cs_root_sig_.get());                                                                  
                                 clist_[idx]->SetGraphicsRootSignature(cs_root_sig_.get());                                                                  
                                 
                                  
                                 ID3D12DescriptorHeap* ppHeaps[] = { cbheap_[idx].get() };
	                             clist_[idx]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        
                                                              
                                 
                                 //clist_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());                                 
                                 clist_[idx]->SetGraphicsRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());
                                 ////clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_.get()->GetGPUDescriptorHandleForHeapStart());                                                                                                   
                                 //clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_[idx]->GetGPUDescriptorHandleForHeapStart());
                                 
                                 clist_[idx]->RSSetViewports(1, *viewports.begin());
	                             clist_[idx]->RSSetScissorRects(1, &scissor_);    



	                             clist_[idx]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	                             clist_[idx]->OMSetRenderTargets(0, nullptr, TRUE, nullptr);    
                                 //clist_[idx]->DrawInstanced(14, 1, 0, 0);
                                    
                                 //clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                 //         ubuffers[idx].get(), 
                                 //         D3D12_RESOURCE_STATE_RENDER_TARGET, 
                                 //         D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

                                 //clist_[idx]->Close();

                                 //
                                 //clist_[idx]->Reset(cs_alloc_[idx].get(), pso_cs_.get());                                 
                                 //clist_[idx]->SetComputeRootSignature(root_sig_.get());
                                 
                                 //ID3D12DescriptorHeap* ppHeaps_b[] = { ubuffers[idx].uav_heap() };
	                             //clist_[idx]->SetDescriptorHeaps(_countof(ppHeaps_b), ppHeaps_b);        
                                 //cs_clist_[idx]->SetComputeRootConstantBufferView(0, nullptr);
                                 //cs_clist_[idx]->SetComputeRootDescriptorTable(1, nullptr);
                                 //cs_clist_[idx]->SetComputeRootDescriptorTable(2, nullptr);
                                 //clist_[idx]->SetComputeRootDescriptorTable(3, ubuffers[idx].uav_heap()->GetGPUDescriptorHandleForHeapStart());
                                 //clist_[idx]->Dispatch(24,16,1);   
                                 clist_[idx]->DrawInstanced(3, 1, 0, 0);
                                 clist_[idx]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                          ubuffers[idx].get(), 
                                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
                                          D3D12_RESOURCE_STATE_COPY_SOURCE));
                                 //clist_[idx]->Close();

                                 //
                                 
                                 //clist_[idx]->Reset(calloc_[idx].get(), pso_.get());                                 
                                 //clist_[idx]->SetGraphicsRootSignature(root_sig_.get());

                                 gtl::d3d::graphics_command_lists::skybox_graphics_command_list_second(
                                     clist_[idx],swchain_.get_current_resource(), ubuffers[idx]);

                    

                                 std::initializer_list<ID3D12CommandList*> clists{clist_[idx].get()};                                         
                                 if (WaitForSingleObject(swchain_->GetFrameLatencyWaitableObject(),0) == WAIT_OBJECT_0) {                                                         	                    
                                     cqueue_->ExecuteCommandLists(gtl::win::array_size(clists), clists.begin());
                                     swchain_->Present(0,0);
                                     return true;
                                 }            
                                 return false; 
                               };
            single_pass(0);
            wait_for_gpu(dev_,cqueue_);

            //sync_(compute_pass, [](){});
        }
        
        } // dummy scope just to check live objects at exit        
        report_live_objects(dev_);

        */
