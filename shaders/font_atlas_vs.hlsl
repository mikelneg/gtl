/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

//  SDF font renderer

struct root_constants_ {
    float4 viewport;
    float font_scale;
};

ConstantBuffer<root_constants_> root_constants : register(b0, space1);

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
};

struct ps_output {
    float4 color : SV_Target0;
};

float screen_ratio()
{
    return 960.0f / 540.0f;
    //return root_constants.viewport_wh.x / root_constants.viewport_wh.y;
}

float viewport_ratio()
{
    return root_constants.viewport.z / root_constants.viewport.w;
}

vs_output vs_main(uint vertex_id
                  : SV_VertexID, vs_input input)
{
    vs_output output;

    output.position = input.position;
    output.position.z = 1.0f;
    output.position.w = 1.0f;

    output.uv = input.uv; // 415x512

    uint width, height, levels;
    texture1.GetDimensions(0, width, height, levels);

    //float ratio = screen_viewport.x / screen_viewport.y;

    //output.position.xy = root_constants.viewport_upper_left_xy + (input.position * (1.0f / float2(root_constants.viewport_wh)));
    //output.position.xy += root_constants.viewport_upper_left_xy;

    float2 transform = 2.0f / root_constants.viewport.zw;

    //output.position.y = -output.position.y;

    output.position.xy *= root_constants.font_scale;

    output.position.xy *= transform;
    output.position.xy -= 1.0f;
    //output.position.xy *= 1.0f / float2(root_constants.viewport_wh); // screen_ratio()*viewport_ratio();
    output.position.y = -output.position.y;

    //output.position.xy += (transform * root_constants.viewport_upper_left_xy) - 1.0f;

    output.uv.xy *= 1.0f / float2(width, height);
    //output.uv.xy = output.position.xy;

    return output;
}

ps_output ps_main(vs_output input)
{
    ps_output output;
    //output.color = float4(1.0,0.0f,0.5f,1.0f);

    float alpha = texture1.Sample(font_atlas_sampler, input.uv.xy).a;
    //
    float delta = 0.3f; // ignores the delta and just uses alpha
    //
    clip(alpha - (0.5f - delta));

    output.color = float4(float3(1.0f, 1.0f, 1.0f), smoothstep(0.5f - delta, 0.5f + delta, alpha));
    //output.color = float4(smoothstep(0.5f-delta, 0.5f+delta, alpha), 0.0f, 0.0f, 1.0f);
    //output.color = float4(float3(1.0f, 1.0f, 1.0f), alpha);

    //output.color = texture1.Sample(font_atlas_sampler, input.uv.xy);

    //output.color *= float4(1.0f, 1.0f, 1.0f, 0.5f);
    //output.color.r = output.color.a;
    //output.color.gb = 0.0f;
    //
    //if (output.color.a > 0.0f && output.color.a < 1.0f)
    //    output.color.g = 1.0f;
    //
    //output.color.a = 1.0f;
    //output.color += float4(0.4f, 0.0f, 0.0f, 1.0f)

    return output;
}

//[maxvertexcount(4)]
//void GShader(uint instance_id : SV_PrimitiveID, inout PointStream<Particle> outstream)
//{
//    Particle pout = (Particle)0;
//
//    float  d = distance(input[0].pos.xyz, attractor.xyz);
//    float3 nv = ((attractor.xyz - input[0].pos.xyz) * gravity.x) / (d);
//
//    //pout.pos.xyz = input[0].pos.xyz + ((input[0].vel.xyz + nv) * elapsedtime);
//    pout.vel.xyz = ((input[0].vel.xyz * gravity.y) + (nv * elapsedtime));
//    //pout.pos.xyz = input[0].pos.xyz + (input[0].vel.xyz * elapsedtime) + (nv *elapsedtime);
//    pout.pos.xyz = input[0].pos.xyz + (pout.vel.xyz * elapsedtime);
//
//
//    pout.pos.w = input[0].pos.w;
//    pout.vel.w = input[0].vel.w;
//
//    float a = atan2(input[0].vel.y, input[0].vel.x) - PI2;
//
//    pout.orientation.xy = 0;           // this allows rotation only around z-axis
//    pout.orientation.z = sin(a / 2);
//    pout.orientation.w = cos(a / 2);
//
//    outstream.Append(pout);
//}
//
//////////////////////////////////////
//GeometryShader GSStreamOut =
//ConstructGSWithSO(CompileShader(gs_4_0, GShader()),
//                  "POSITION.xyzw; VELOCITY.xyzw; ORIENTATION.xyzw");
//
//
//////////////////////////////////////
//float4 SimplePShader(ParticleOut input) : SV_TARGET
//{
//    //input.color;
//    //	return float4((input.color.z * 0.0004)+0.008,
//    //				  (input.color.z * 0.0004)+0.01,
//    //				  (input.color.z * 0.0004)+0.02,
//    //				  1);
//    //
//    return float4((input.color.z * 0.0004)+0.01,
//    (input.color.y * 0.00004)+0.01,
//                  (input.color.y * 0.00004)+0.02,
//                  1);
//
////float4(saturate(input.color.z * 0.004),
////	  saturate(input.color.z * 0.004)+0.01,
////	  0.01,
/////	  0.2);
//}
//
