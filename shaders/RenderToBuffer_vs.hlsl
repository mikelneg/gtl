#include "ConstantBuffers.hlsl"

Texture2D texture_ : register(t0);
//Texture2D glow_ : register(t1);
//TextureCube texture_ : register( t0 );

SamplerState RenderToBufferSampler : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

struct VS_Out {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_Out VS_FullscreenTriangle(in uint vertex_id
                             : SV_VertexID)
{
    VS_Out result;

    result.position.x = (vertex_id / 2) * 4.0 - 1.0;
    result.position.y = (vertex_id % 2) * 4.0 - 1.0;
    result.position.z = 1.0; // test
    result.position.w = 1.0;

    result.uv.x = (vertex_id / 2) * 2.0;
    result.uv.y = 1.0 - (vertex_id % 2) * 2;

    return result;
}

float4 PS_FullscreenTriangle(VS_Out input)
    : SV_TARGET0
{
    //float4 color = texture_.Sample(RenderToBufferSampler, input.uv);
    //float4 glow = glow_.Sample(RenderToBufferSampler,input.uv);

    //color *= glow.a;

    return texture_.Sample(RenderToBufferSampler, input.uv);
}
