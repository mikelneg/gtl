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
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct Index {
    uint idx;
};

struct OutputType {
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct ps_output {
    float4 color : SV_Target0;
    uint id : SV_Target1;
};

cbuffer per_frame : register(b0)
{
    float4x4 view;
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

    float L = viewport.x; 
    float R = viewport.x + viewport.z;
    float B = viewport.y + viewport.w;
    float T = viewport.y;
    float4x4 mvp = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };

    //output_.pos = float4((v.position*2.0f)-1.0f,1.0f,1.0f);

    output_.pos = mul(float4(input.pos, 0.0f, 1.0f), mvp);
    
    output_.uv = input.uv;
    output_.color = input.color;
    return output_;
}

ps_output ps_main(OutputType input)
{
    ps_output out_;

    out_.color = input.color * color_texture_.Sample(sampler_, input.uv).r;
    out_.id = 0;
    return out_;
}