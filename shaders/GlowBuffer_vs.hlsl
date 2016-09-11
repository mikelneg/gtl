#include "ConstantBuffers.hlsl"

Texture2D texture_ : register(t0);
Texture2D glow_ : register(t1);
//TextureCube texture_ : register( t0 );

SamplerState RenderToBufferSampler : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
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
    float4 color = texture_.Sample(RenderToBufferSampler, input.uv);
    float4 glow = glow_.Sample(RenderToBufferSampler, input.uv);

    //color = color + glow * pow(glow.a, 2);
    color = 0.8f * color + 0.4f * 2 * color * glow * glow.a + 0.4f * glow * glow.a;
    //color = glow.a;

    //color = 0.5f * color + 0.5f * glow.a * glow;

    return color;
}
