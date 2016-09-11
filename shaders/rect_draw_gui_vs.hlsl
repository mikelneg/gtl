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

//struct root_constants_ {
//    float4 viewport;
//    float font_scale;
//};
//
//ConstantBuffer<root_constants_> root_constants : register(b0,space1);

Texture2D texture1 : register(t0);

SamplerState font_atlas_sampler : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

struct vs_input {
    float4 position : POSITION;
    float4 uv : UV;
};

struct vs_output {
    float4 position : SV_Position;
    float4 uv : COLOR;
    uint id : TEXCOORD;
};

struct ps_output {
    float4 color : SV_Target0;
    uint id : SV_Target1;
};

vs_output vs_main(uint vertex_id
                  : SV_VertexID, vs_input input)
{
    vs_output output;

    output.position = input.position;
    output.position.z = 1.0f;
    output.position.w = 1.0f;

    output.uv = input.uv;
    output.id = vertex_id / 6;

    return output;
}

ps_output ps_main(vs_output input)
{
    ps_output output;
    output.color = texture1.Sample(font_atlas_sampler, input.uv.xy);
    output.id = input.id;
    return output;
}

///////////
