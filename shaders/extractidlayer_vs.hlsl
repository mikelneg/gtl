#include "ConstantBuffers.hlsl"

Texture2D<uint> texture_ : register(t0);

SamplerState textureSampler_ : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

float4 extractVShader(uint id
                      : SV_VertexID)
    : SV_POSITION
{
    return mouseinfo;
}

uint extractPShader(nointerpolation float4 input
                    : SV_POSITION)
    : SV_TARGET0
{
    return uint(55); //texture_.Load(input.xyz);
}