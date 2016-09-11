/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

struct vs_output {
    noperspective float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D texture_ : register(t0);

SamplerState sampler_ : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

vs_output vs_main(in uint v_id
                  : SV_VertexID)
{    
    vs_output output_;

    // From Timothy Lottes FXAA vertex shader:
    output_.uv = float2((v_id << 1) & 2, v_id & 2);
    output_.pos = float4(output_.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

    return output_;
}

float4 ps_main(noperspective vs_output input)
    : SV_Target0
{
    return float4(texture_.Sample(sampler_, input.uv).rgb, 1.0f);
}