/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

struct InputBufferType {
    float4 xywh;
    float4 rgb_angle;
};

struct OutputBufferType {
    float4 xyzw;
    float4 color;
};

ConsumeStructuredBuffer<InputBufferType> billboards : register(u0);
AppendStructuredBuffer<OutputBufferType> output_vertices : register(u1);

[numthreads(1, 1, 1)] void billboard_main(uint3 DTid
                                          : SV_DispatchThreadID)
{
    BufferOut[DTid.x].i = Buffer0[DTid.x].i + Buffer1[DTid.x].i;
    BufferOut[DTid.x].f = Buffer0[DTid.x].f + Buffer1[DTid.x].f;
#ifdef TEST_DOUBLE
    BufferOut[DTid.x].d = Buffer0[DTid.x].d + Buffer1[DTid.x].d;
#endif
}

ConsumeStructuredBuffer cs_input{

};

RWTexture2D<float4> target_texture : register(u0);
TextureCube texture_resource : register(t0);
SamplerState sampler_ : register(s0);

float3x3 sharpen_kernel()
{
    float3x3 mat = {0.0f, -1.0f, 0.0f,
                    -1.0f, 5.0f, -1.0f,
                    0.0f, -1.0f, 0.0f};
    return mat;
}

float3x3 gaussian_blur_kernel()
{
    float3x3 mat = {1.0f, 2.0f, 1.0f,
                    2.0f, 4.0f, 2.0f,
                    1.0f, 2.0f, 1.0f};
    return mat * 0.0625; // 1/16
}

uint data;

[numthreads(8, 8, 1)] void cs_main(uint3 thread_id
                                   : SV_DispatchThreadID, uint3 group_thread_id
                                   : SV_GroupThreadID, uint3 group_id
                                   : SV_GroupId, uint group_index
                                   : SV_GroupIndex)
{
    float2 p = thread_id.xy;
    float3x3 filt = gaussian_blur_kernel();

    //// groupshared
    ////float4 color = target_texture[p];
    float4 color = float4(0, 0, 0, 0);
    //
    color += target_texture.Load(p + float2(-1, -1)) * filt[0][0];
    color += target_texture.Load(p + float2(0, -1)) * filt[1][0];
    color += target_texture.Load(p + float2(1, -1)) * filt[2][0];
    color += target_texture.Load(p + float2(-1, 0)) * filt[0][1];
    color += target_texture.Load(p) * filt[1][1];
    color += target_texture.Load(p + float2(1, 0)) * filt[2][1];
    color += target_texture.Load(p + float2(-1, 1)) * filt[0][2];
    color += target_texture.Load(p + float2(0, 1)) * filt[1][2];
    color += target_texture.Load(p + float2(1, 1)) * filt[2][2];

    //color += target_texture[p  + float2(-1,-1)] * filt[0][0];
    //color += target_texture[p  + float2(0,-1)]  * filt[1][0];
    //color += target_texture[p  + float2(1,-1)]  * filt[2][0];
    //color += target_texture[p  + float2(-1,0)]  * filt[0][1];
    //color += target_texture[p                ]  * filt[1][1];
    //color += target_texture[p  +  float2(1,0)]  * filt[2][1];
    //color += target_texture[p  + float2(-1,1)]  * filt[0][2];
    //color += target_texture[p  +  float2(0,1)]  * filt[1][2];
    //color += target_texture[p  +  float2(1,1)]  * filt[2][2];

    //target_texture[p  + float2(-1,-1)] = (1.0f - filt[0][0]) * target_texture[p  + float2(-1,-1)] + color * filt[0][0];
    //target_texture[p  + float2(0,-1)]  = (1.0f - filt[1][0]) * target_texture[p  + float2(0,-1)]  + color * filt[1][0];
    //target_texture[p  + float2(1,-1)]  = (1.0f - filt[2][0]) * target_texture[p  + float2(1,-1)]  + color * filt[2][0];
    //target_texture[p  + float2(-1,0)]  = (1.0f - filt[0][1]) * target_texture[p  + float2(-1,0)]  + color * filt[0][1];
    //target_texture[p                ]  *= 1.02f;
    //target_texture[p  +  float2(1,0)]  = (1.0f - filt[2][1]) * target_texture[p  +  float2(1,0)]  + color * filt[2][1];
    //target_texture[p  + float2(-1,1)]  = (1.0f - filt[0][2]) * target_texture[p  + float2(-1,1)]  + color * filt[0][2];
    //target_texture[p  +  float2(0,1)]  = (1.0f - filt[1][2]) * target_texture[p  +  float2(0,1)]  + color * filt[1][2];
    //target_texture[p  +  float2(1,1)]  = (1.0f - filt[2][2]) * target_texture[p  +  float2(1,1)]  + color * filt[2][2];

    //GroupMemoryBarrierWithGroupSync();
    //
    //target_texture[p] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    //
    //target_texture[p] = float4(group_thread_id.x / 8.0f, group_thread_id.y / 8.0f, 0.01f,1.0f);
    //
    //InterlockedAdd(data, 1);
    //
    //GroupMemoryBarrierWithGroupSync();
    //
    //target_texture[p] = float4(data / 64.0f, group_thread_id.y / 8.0f, 0.0f, 1.0f);

    //    target_texture[p] *= 0.5f;

    //target_texture[p] = float4(group_id.x / 107.0f, group_id.y / 60.0f, 1.0f, 1.0f) * color + float4(0.01f, 0.01f, 0.0f, 0.0f);
    target_texture[p] = color;
}

struct vs_out {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

vs_out vs_main(in uint vertex_id
               : SV_VertexID)
{
    vs_out result;

    result.position.x = (vertex_id / 2) * 4.0 - 1.0;
    result.position.y = (vertex_id % 2) * 4.0 - 1.0;
    result.position.z = 1.0; // test
    result.position.w = 1.0;

    result.uv.x = (vertex_id / 2) * 2.0;
    result.uv.y = 1.0 - (vertex_id % 2) * 2;

    return result;
}

float4 ps_main(vs_out input)
    : SV_Target0
{
    //target_texture[input.position.xy] = float4(input.position.x / 960.0f, input.position.y / 540.0f, 0.0f, 1.0f);
    //target_texture[input.position.xy] = float4(input.uv.x, input.uv.y, input.position.x / 960.0f, 1.0f);
    //target_texture[input.position.xy] = texture_resource.Sample(sampler_, float3(input.uv.xy,1));
    //return float4(1.0f,0,1.0f,1.0f);
    return texture_resource.Sample(sampler_, float3(input.uv.xy, 1));
}
