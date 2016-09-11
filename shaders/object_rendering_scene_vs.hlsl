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

struct VertexInputType {
    float4 position : VERTEX_POSITION;
    float4 normal : VERTEX_NORMAL;
    uint4 bone_ids : VERTEX_BONE_IDS;
    float4 bone_weights : VERTEX_BONE_WEIGHTS;
};

struct InstanceInputType {
    uint4 info : INSTANCE_INFO;
    // <bone_offset, material_id, entity_id, mesh_id>
};

struct OutputType {
    float4 pos : SV_Position;
    float4 color : COLOR;
    uint id : OBJECT_ID;
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

struct bone {
    float4x4 t;
};

StructuredBuffer<bone> bones_ : register(t1, space2);

Texture2D color_texture_ : register(t0);

SamplerState sampler_ : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

OutputType vs_main(in uint instance_id
                   : SV_InstanceID, VertexInputType input_, InstanceInputType instance_)
{
    OutputType output_ = (OutputType)0;

    float4 pos = 0;
    float3 norm = 0;

    for (uint i = 0; i < bone_count; ++i)
    {
        pos += mul(input_.position, bones_[instance_.info.x + input_.bone_ids[i]].t) * input_.bone_weights[i];
        norm += mul(input_.normal.xyz, (float3x3)bones_[instance_.info.x + input_.bone_ids[i]].t) * input_.bone_weights[i];
        //pos += mul(input_.position,bones_[instance_.info.x+i].t) * input_.bone_weights[i];
        //norm += mul(input_.normal.xyz,(float3x3)bones_[instance_.info.x+i].t) * input_.bone_weights[i];
    }

    output_.pos = mul(pos, view);

    output_.pos.y *= screen_ratio();

    //output_.color = float4(0.3f,0.0f,0.3f,1.0f) +
    //                float4(1.0f,0.0f,0.0f,1.0f) * input_.bone_weights[0] +
    //                float4(0.0f,1.0f,0.0f,1.0f) * input_.bone_weights[1] +
    //                float4(0.7f,0.7f,0.0f,1.0f) * input_.bone_weights[2] +
    //                float4(0.0f,0.0f,1.0f,1.0f) * input_.bone_weights[3];
    //

    output_.color.rgb = (norm + 1.0f) / 2.0f; // for normals
    //output_.color.rgb = (normalize(input_.position.xyz) + 1.0f) / 2.0f; // for vertices
    //output_.color.r = max(dot(float3(0.0f,0.0f,-1.0f),norm),0.0f);
    //output_.color.g = max(dot(float3(0.0f,-1.0f,0.0f),norm),0.0f);
    //output_.color.b = max(dot(float3(-1.0f,0.0f,0.0f),norm),0.0f);
    output_.color.a = 1.0f;

    output_.id = instance_.info.z;

    return output_;
}

ps_output ps_main(OutputType input)
{
    ps_output out_;

    out_.color = input.color;
    out_.id = input.id;

    return out_;
}