#ifndef BXCVCVBOIWWR_GTL_SCENES_DEMO_TRANSITION_H_
#define BXCVCVBOIWWR_GTL_SCENES_DEMO_TRANSITION_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
        
-----------------------------------------------------------------------------*/

#include <gtl/include/events.h>
#include <gtl/include/keyboard_enum.h>

#include <Windows.h>

#include <cstddef>
#include <array>
#include <utility>

#include <gtl/include/gtl_window.h>
#include <gtl/include/d3d_types.h>
#include <gtl/include/d3d_funcs.h>

//#include <gtl/include/synchronized.h>
#include <gtl/include/clist_skybox.h>

#include <gtl/include/swirl_effect_transition_scene.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <cmath>

namespace gtl {
namespace scenes {
namespace transitions {

 

    class twinkle_effect {        
        
        constexpr static std::size_t frame_count = 3; // TODO place elsewhere..

        gtl::d3d::device& dev_;
        gtl::d3d::command_queue& cqueue_;
        gtl::d3d::swap_chain& swchain_;

        std::array<gtl::d3d::resource_descriptor_heap,frame_count> mutable cbheap_;
        gtl::d3d::sampler_descriptor_heap sampler_heap_;
        
        std::array<gtl::d3d::direct_command_allocator,frame_count> mutable calloc_;             
        std::array<gtl::d3d::direct_command_allocator,frame_count> mutable cs_alloc_;             

        gtl::d3d::root_signature root_sig_;
        gtl::d3d::root_signature cs_root_sig_;
        gtl::d3d::vertex_shader vshader_;
        gtl::d3d::pixel_shader pshader_;
        gtl::d3d::compute_shader cshader_;
        gtl::d3d::vertex_shader csvshader_;
        gtl::d3d::pixel_shader cspshader_;
        gtl::d3d::pipeline_state_object pso_;
        gtl::d3d::pipeline_state_object cs_pso_;
        
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable clist_;       
        std::array<gtl::d3d::graphics_command_list,frame_count> mutable cs_list_;
                
        gtl::d3d::D3D12Viewport mutable viewport_;
        gtl::d3d::D3D12ScissorRect scissor_;    
                      
        gtl::d3d::D3D12Viewport mutable viewport1_;
        gtl::d3d::D3D12Viewport mutable viewport2_;

        cbuffer mutable cbuf_;
                
        std::pair<char*,unsigned> mutable cbuf_data;        

        std::array<gtl::d3d::constant_buffer,frame_count> mutable cbuffer_;
                                                     
        std::array<gtl::d3d::CpuDescriptorHandle,frame_count> mutable handles;
                
        gtl::d3d::srv texture_;
        
        std::array<gtl::d3d::uav_texture2D, 3> mutable ubuffers;
       
        gtl::d3d::rtv_descriptor_heap uav_rtv;
        gtl::d3d::sampler sampler_;
        //gtl::d3d::synchronized sync_;

    public:
        //twinkle_effect() = default;        
        template <typename T>
        void load_text(T idx) const {        
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
                                 }

        template <typename T>
        bool single_pass(T idx) const {        
                                 cs_alloc_[idx]->Reset(); 
                                 cs_list_[idx]->Reset(cs_alloc_[idx].get(), cs_pso_.get());

                                 cs_list_[idx]->SetComputeRootSignature(cs_root_sig_.get());                                                                  
                                  
                                 ID3D12DescriptorHeap* ppHeaps[] = { sampler_heap_.get(), cbheap_[idx].get() };
	                             cs_list_[idx]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);        

                                 //clist_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());                                                                  
                                 cs_list_[idx]->SetComputeRootDescriptorTable(0, cbheap_[idx].get()->GetGPUDescriptorHandleForHeapStart());
                                 ////clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_.get()->GetGPUDescriptorHandleForHeapStart());                                                                                                   
                                 //clist_[idx]->SetGraphicsRootDescriptorTable(2, cbheap_[idx]->GetGPUDescriptorHandleForHeapStart());     
                                 
                                 cs_list_[idx]->Dispatch(4,68,1);                                    
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
                                 }

        twinkle_effect(gtl::d3d::device& dev_, gtl::d3d::command_queue& cqueue_, gtl::d3d::swap_chain& swchain_) // TODO temporary effect..
            :   dev_{dev_}, 
                cqueue_{cqueue_}, 
                swchain_{swchain_},
                cbheap_{{{dev_,2,gtl::d3d::tags::shader_visible{}},
                         {dev_,2,gtl::d3d::tags::shader_visible{}},
                         {dev_,2,gtl::d3d::tags::shader_visible{}}}},
                sampler_heap_{dev_,1},
                calloc_{{{dev_},{dev_},{dev_}}},
                cs_alloc_{{{dev_},{dev_},{dev_}}},             
                root_sig_{dev_, gtl::d3d::dummy_rootsig_1()},
                cs_root_sig_{dev_, gtl::d3d::dummy_rootsig_2()},
                vshader_{L"skybox_vs.cso"},
                pshader_{L"skybox_ps.cso"},            
                cshader_{L"demo_cs_cs.cso"},
                csvshader_{L"demo_cs_vs.cso"},
                cspshader_{L"demo_cs_ps.cso"},
                pso_{dev_, cs_root_sig_, csvshader_, cspshader_},
                cs_pso_{dev_, cs_root_sig_, cshader_},            
                clist_{{{dev_,calloc_[0],pso_},{dev_,calloc_[1],pso_},{dev_,calloc_[2],pso_}}},            
                cs_list_{{{dev_,cs_alloc_[0],cs_pso_},{dev_,cs_alloc_[1],cs_pso_},{dev_,cs_alloc_[2],cs_pso_}}},
                viewport_{0.0f,0.0f,960.0f,540.0f,0.0f,1.0f},
                scissor_{0,0,960,540},                          
                viewport1_{0.0f,0.0f,320.0f, 240.0f, 0.0f, 1.0f},
                viewport2_{322.0f,0.0f,960.0f,240.0f, 0.0f, 1.0f},
                cbuf_{},
                cbuf_data{reinterpret_cast<char*>(&cbuf_), static_cast<unsigned>(sizeof(cbuf_))},
                cbuffer_{{{dev_,cbheap_[0],cbuf_data},{dev_,cbheap_[0],cbuf_data},{dev_,cbheap_[0],cbuf_data}}},
                handles{{   {cbheap_[0]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[0].increment_value()},
                            {cbheap_[1]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[1].increment_value()},
                            {cbheap_[2]->GetCPUDescriptorHandleForHeapStart(),0,cbheap_[2].increment_value()} }},        
                    
                texture_{dev_,{handles[0],handles[1],handles[2]},cqueue_,L"D:\\images\\skyboxes\\Nightsky.dds"},                
                ubuffers{{{swchain_,handles[0].Offset(1, cbheap_[1].increment_value())},
                          {swchain_,handles[1].Offset(1, cbheap_[1].increment_value())},
                          {swchain_,handles[2].Offset(1, cbheap_[1].increment_value())}}},      
                uav_rtv{dev_,1},            
                sampler_{dev_,sampler_heap_->GetCPUDescriptorHandleForHeapStart()}
                //sync_{cqueue_,gtl::d3d::fence{dev_}, 2, 2}; // framecount - 1
        {
            dev_->CreateRenderTargetView(ubuffers[0].get(), nullptr, uav_rtv->GetCPUDescriptorHandleForHeapStart());                                
            load_text(0);
            wait_for_gpu(dev_,cqueue_);
            std::cout << "new twinkle_effect...\n";
            //single_pass(0);                                   
            //wait_for_gpu(dev_,cqueue_);
        }

        twinkle_effect& operator=(twinkle_effect&&) { return *this; }
        twinkle_effect(twinkle_effect&&) = default;

        ~twinkle_effect() {
            std::cout << "~twinkle_effect()\n";
        }

        void draw(float f) const {
            single_pass(0);
            wait_for_gpu(dev_,cqueue_);
        }
        
        template <typename YieldType>
        gtl::event handle_events(YieldType& yield) const {
            // TODO this will probably never be called, but it needs to be sorted out how the transition handler works with regards to transition effects..             
            namespace ev = gtl::events;
            int count{};
            while (!same_type(yield().get(),ev::exit_immediately{})){                   
                if (same_type(yield.get(),ev::keydown{})){ 
                    if ( boost::get<ev::keydown>( yield.get().value() ).key == gtl::keyboard::Q) {
                        std::cout << "twinkle_effect(): q pressed, exiting A from route 0 (none == " << count << ")\n";                                                                
                        return gtl::events::exit_state{0};
                    } else 
                    if ( boost::get<ev::keydown>( yield.get().value() ).key == gtl::keyboard::K) {
                        std::cout << "twinkle_effect(): k pressed, throwing (none == " << count << ")\n";                                                
                        throw std::runtime_error{__func__};
                        //return 1;
                    } else { std::cout << "twinkle_effect(): some key pressed..\n"; }
                } else if (same_type(yield.get(),ev::none{})) {
                    count++;
                 }
            }            
            return gtl::events::exit_state{0};
        }            
    };

}}} // namespaces
#endif


/*
#include <gtl/include/gtl_window.h>
#include <gtl/include/d3d_types.h>
#include <gtl/include/d3d_funcs.h>

#include <gtl/include/synchronized.h>

#include <gtl/include/clist_skybox.h>

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