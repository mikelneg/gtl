/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

cbuffer root_constants : register(b0, space1)
{
    float4 viewport; // top_left_x, top_left_y, width, height
};

float screen_ratio()
{
    return viewport.z / viewport.w;
}

struct Vertex {
    float4 pos : POSITION;    
    float4 color : COLOR;
};

struct Index {
    uint idx;
};

struct OutputType {
    float4 pos : SV_Position;
    float4 color : COLOR;    
};

struct ps_output {
    float4 color : SV_Target0;
    uint id : SV_Target1;
};

cbuffer per_frame : register(b0)
{
    float4x4 view;
    float4x4 proj;
};

cbuffer per_frame_also : register(b0, space2)
{
    uint bone_count;
};

//StructuredBuffer<Vertex> vertices_ : register(t1, space2);
//StructuredBuffer<Index> indices_ : register(t2, space2);

Texture2D color_texture_ : register(t0);

SamplerState sampler_ : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

OutputType vs_main(in uint v_id
                   : SV_VertexID, Vertex input)
{
    OutputType output_ = (OutputType)0;

     static const float4x4 swap_z_y = {1.0f, 0.0f, 0.0f, 0.0f, 
                                       0.0f, 0.0f, 1.0f, 0.0f,
                                       0.0f, 1.0f, 0.0f, 0.0f,
                                       0.0f, 0.0f, 0.0f, 1.0f};
    
    static const float4x4 trans = mul(view,mul(proj,swap_z_y));        

    output_.pos = mul(trans,input.pos);            
    output_.pos.y *= screen_ratio();
    output_.color = input.color;    
    return output_;
}

ps_output ps_main(OutputType input)
{
    ps_output out_;

    out_.color = input.color; 
    out_.id = 0;
    return out_;
}