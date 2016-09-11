/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

struct InputType {
    float4 xywh : POSITION;
    float4 uv_angle : COLOR_ANGLE; // I was converting this to UV, but haven't finished yet..
    uint id : GUID;
};

struct OutputType {
    float4 pos : SV_Position;
    float4 color : COLOR;
    uint id : GUID;
};

struct ps_output {
    float4 color : SV_Target0;
    uint id : SV_Target1;
};

cbuffer per_frame : register(b0)
{
    float4x4 view;
};

Texture2D color_texture_ : register(t0);

SamplerState sampler_ : register(s0)
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

static float4x2 box = {{-1.0f, 1.0f}, // triangle strip version
                       {1.0f, 1.0f},
                       {-1.0f, -1.0f},
                       {1.0f, -1.0f}};

//static float4x2 box = {{ -1.0f,  1.0f},       // line/point version
//                       {  1.0f,  1.0f},
//                       {  1.0f, -1.0f},
//                       { -1.0f, -1.0f}};

InputType vs_main(InputType input)
{
    return input;
}

float screen_ratio()
{
    return 960.0f / 540.0f;
}

// would be 4 if triangl strips..
//[maxvertexcount(5)] // pointstream
[maxvertexcount(4)] void gs_main(point InputType input[1], inout TriangleStream<OutputType> outstream)
//void gs_main(point InputType input[1], inout LineStream<OutputType> outstream)
{
    OutputType out_ = (OutputType)0;

    const float w = input[0].uv_angle.w;

    //const float4x4 rotmat = {{ cos(w), sin(w), 0.0f, 0.0f},
    //                         {-sin(w), cos(w), 0.0f, 0.0f},
    //                         { 0.0f, 0.0f, 0.0f, 0.0f},
    //                         { 0.0f, 0.0f, 0.0f, 0.0f}};

    const float2x2 rotmat = {{cos(w), sin(w)},
                             {-sin(w), cos(w)}};

    //outstream.Append(out_);

    for (int i = 0; i < 4; ++i)
    {
        //out_.pos = mul(float4(input[0].xywh.xy,0.0f,1.0f) + (box[i] * input[0].xywh.zw), rotmat);
        //out_.pos = mul(float4(input[0].xywh.xy,0.0f,1.0f), rotmat);
        //out_.pos = float4((input[0].xywh.xy * 0.01f) + mul(box[i] * input[0].xywh.zw, rotmat),1.0f,1.0f);
        out_.pos = float4((input[0].xywh.xy) + mul(box[i] * input[0].xywh.zw, rotmat), 1.0f, 1.0f);

        out_.pos = mul(out_.pos, view);
        out_.pos.y *= screen_ratio();
        out_.color = float4(input[0].uv_angle.rgb, 1.0f);
        out_.pos.z = out_.pos.w;
        out_.id = input[0].id;
        outstream.Append(out_);
    }

    //out_.pos = float4(input[0].xywh.xy + mul(box[0]  * input[0].xywh.zw, rotmat),1.0f,1.0f);
    //out_.color = float4(input[0].rgb_angle.rgb, 1.0f);
    //outstream.Append(out_);

    // remove above ^ for triangles, and add below

    outstream.RestartStrip();
}

ps_output ps_main(OutputType input)
{
    ps_output out_;
    out_.color = //color_texture_.Sample(sampler_,input.color.xy);
        out_.color = input.color;
    out_.id = input.id;
    return out_;
}