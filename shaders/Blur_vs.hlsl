#include "ConstantBuffers.hlsl"

Texture2D texture_ : register(t0);

SamplerState BlurSampler : register(s0)
{
    Filter = ANISOTROPIC;
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
    result.position.z = 0;
    result.position.w = 1;

    result.uv.x = (vertex_id / 2) * 2.0;
    result.uv.y = 1.0 - (vertex_id % 2) * 2;

    return result;
}

//float4 PS_HorizontalBlurPass( VS_Out input ) : SV_TARGET0
float4 PS_HorizontalBlurPass(VS_Out input)
    : SV_TARGET
{
    static float2 blursize = float2(1.0f / screen_viewport.x, 1.0f / screen_viewport.y);
    static int count = 32;

    float4 color = float4(0, 0, 0, 0);

    float2 offset = ((input.position.xy / 640.0f) - 0.5f) * 2.0f; // -1 to 1

    float2 uv = input.uv;

    float2 step = float2(blursize.x, 0.0f) * 2;

    //float2 step = float2(blursize.x, 0.0f) * 14.5f;

    for (float i = 0.5f; i > 0.0f; i -= (1.0f / count))
    {
        color += texture_.Sample(BlurSampler, uv) * (0.25f * i);
        uv -= step;
    }

    uv = input.uv;

    for (i = 0.5f; i > 0.0f; i -= (1.0f / count))
    {
        color += texture_.Sample(BlurSampler, uv) * (0.25f * i);
        uv += step;
    }
    //for (int i = count-1; i > 0; --i) {
    //    color += texture_.Sample(BlurSampler, uv) * ((count - float(i)) / count); // * (1.0f - (pow((1.0f * i) / count, 2)/(pow((1.0f * i) / count, 2)+0.01f)));
    //    uv += step;
    //}

    return color;
}

//float4 PS_VerticalBlurPass( VS_Out input ) : SV_TARGET0
float4 PS_VerticalBlurPass(VS_Out input)
    : SV_TARGET
{
    static float2 blursize = float2(1.0f / screen_viewport.x, 1.0f / screen_viewport.y);
    static int count = 32;

    float4 color = float4(0, 0, 0, 0);

    float2 offset = ((input.position.xy / 640.0f) - 0.5f) * 2.0f; // -1 to 1

    float2 uv = input.uv;

    float2 step = float2(0.0f, blursize.y) * 2;

    //for (int i = 1; i < (count / 2); ++i) {
    for (float i = 0.5f; i > 0.0f; i -= (1.0f / count))
    {
        color += texture_.Sample(BlurSampler, uv) * (0.25f * i);
        uv -= step;
    }

    uv = input.uv;

    for (i = 0.5f; i > 0.0f; i -= (1.0f / count))
    {
        color += texture_.Sample(BlurSampler, uv) * (0.25f * i);
        uv += step;
    }

    return color;
}
//  static int count = 8;
//
//  //float2 offset = (input.position - float2(320, 320)) * (1 / 640.0f);
//  float2 offset = -((input.position.xy / 640.0f) - 0.5f) * blursizey;
//
//  color = texture_.Sample(BlurSampler, input.uv);

//color = texture_.Sample(BlurSampler, input.uv) * (1.0f / count);
//
//for (int i = 1; i < count; ++i) {
//    color += texture_.Sample(BlurSampler, input.uv + i * offset * (1.0f / count)) * (1.0f / count);
//}

//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y - 4.0*blursizey)) * 0.07f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y - 3.0*blursizey)) * 0.09f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y - 2.0*blursizey)) * 0.11f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y -     blursizey)) * 0.14f;
//    color += texture_.Sample(BlurSampler,                    input.uv)                    * 0.20f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y + blursizey))     * 0.14f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y + 2.0*blursizey)) * 0.11f;
//	color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y + 3.0*blursizey)) * 0.09f;
//    color += texture_.Sample(BlurSampler, float2(input.uv.x, input.uv.y + 4.0*blursizey)) * 0.07f;

// Something like a "god ray"

///float4 PS_HorizontalBlurPass(VS_Out input) : SV_TARGET
///{
///    float4 color = float4(0,0,0,0);
///    float2 blursize = float2(1/1280.0f, 1/1024.0f);// * (-1 + (input.uv.y*2.0));
///
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x - 4.0*blursizex, input.uv.y)) * 0.07f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x - 3.0*blursizex, input.uv.y)) * 0.09f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x - 2.0*blursizex, input.uv.y)) * 0.11f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x -     blursizex, input.uv.y)) * 0.14f;
///                                                   //color += texture_.Sample(BlurSampler,                                    input.uv)    * 0.20f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x +     blursizex, input.uv.y)) * 0.14f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x + 2.0*blursizex, input.uv.y)) * 0.11f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x + 3.0*blursizex, input.uv.y)) * 0.09f;
///                                                   //color += texture_.Sample(BlurSampler, float2(input.uv.x + 4.0*blursizex, input.uv.y)) * 0.07f;
///
///    static int count = 16;
///
///    float2 offset = ((input.position.xy / 640.0f) - 0.5f) * 2.0f; // -1 to 1
///
///                                                                  //    offset = mouseinfo.xy - offset;
///                                                                  //    offset.y = -offset.y;
///
///    float2 coord = offset;
///
///    offset = -(offset * blursize * 14 * -mouseinfo.x);
///    //offset = normalize(-offset * offset);
///
///
///    float2 uv = input.uv;
///
///    for (int i = 0; i < count; ++i) {
///        //color += texture_.Sample(BlurSampler, input.uv + i * blursizex * offset * (1.0f / count)) * (1.0f / count);
///        //color += texture_.Sample(BlurSampler, uv) * (1.0f * (count - i) / count);
///        color += texture_.Sample(BlurSampler, uv) * (1.0f / count);
///        uv += offset;
///    }
///
///    color.rgb *= color.a;
///
///
///    return color;
///    //return float4(1,1,1,offset.x);
///}
///
/////float4 PS_VerticalBlurPass( VS_Out input ) : SV_TARGET0
///float4 PS_VerticalBlurPass(VS_Out input) : SV_TARGET
///{
///    float4 color = float4(0,0,0,0);
///    static float blursizey = 1 / 3.0f;// * (-1 + (input.uv.x*2.0));
///
///
///    float2 blursize = float2(1/1280.0f, 1/1024.0f);// * (-1 + (input.uv.y*2.0));
///    static int count = 16;
///
///    float2 offset = ((input.position.xy / 640.0f) - 0.5f) * 2.0f; // -1 to 1
///
///                                                                  //offset = mouseinfo.xy - offset;
///
///    float2 coord = offset;
///
///    offset = (offset * blursize * 14 * mouseinfo.y);
///    //offset = normalize(-offset * offset);
///
///
///    float2 uv = input.uv;
///
///    for (int i = 0; i < count; ++i) {
///        //color += texture_.Sample(BlurSampler, input.uv + i * blursizex * offset * (1.0f / count)) * (1.0f / count);
///        //color += texture_.Sample(BlurSampler, uv) * (1.0f * (count - i) / count);
///        color += texture_.Sample(BlurSampler, uv) * (1.0f / count);
///        uv += offset;
///    }
///
///    color.rgb *= color.a;
///
///
///    return color;
///}
