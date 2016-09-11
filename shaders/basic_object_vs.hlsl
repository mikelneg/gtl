#include "ConstantBuffers.hlsl"

////////////////////////////////////
static const float PI = 3.14159265f;
static const float PI2 = PI * 2;

//cbuffer perScene : register ( b1 )
//{
//    extern uint4 meshOffsets[50];
//}

TextureCube envCubeText : register(t0);
TextureCube envCubeText1 : register(t1);
TextureCube envCubeText2 : register(t2);

SamplerState environmentCubeSampler : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

struct BasicVSInput {
    float4 color : COLOR;
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 uv : UV;
};

struct BasicVSInputInstance {
    float4 position : LOCATION;
    float4 orientation : ORIENTATION;
    uint4 various : VARIOUS;
    float4 debugdata : DEBUGDATA;
};

struct BasicVSOutput {
    float4 color : COLOR;
    float4 pos : SV_Position;
    float4 debugdata : COLOR2;
    uint id : ID;
};

struct PSShaderOutput {
    float4 color : SV_TARGET0;
    uint id : SV_TARGET1;
};

//float4x4 rotMatrixFromQuat(float4 quat) {
//
//    return mul(
//           float4x4(quat.w, quat.z, -quat.y, quat.x,
//                   -quat.z, quat.w, quat.x, quat.y,
//                    quat.y, -quat.x, quat.w, quat.z,
//                   -quat.x, -quat.y, -quat.z, quat.w),
//           float4x4(quat.w, quat.z, -quat.y, -quat.x,
//                   -quat.z, quat.w, quat.x, -quat.y,
//                    quat.y, -quat.x, quat.w, -quat.z,
//                    quat.x, quat.y, quat.z, quat.w)
//              );
//}

BasicVSOutput BasicVShader(uint id
                           : SV_VertexID, BasicVSInput input, BasicVSInputInstance instance)
{
    uint leftbound = instance.various.y;
    uint rightbound = instance.various.z;

    if (id < leftbound || id > rightbound)
        return (BasicVSOutput)0;

    BasicVSOutput pout = (BasicVSOutput)0;

    // Faster quaternion multiplication
    // t = 2 * cross(q.xyz, v)
    // v' = v + q.w * t + cross(q.xyz, t)

    // Make a uniform matrix that encapsulates the quaternion rotation

    float3 t = 2 * cross(instance.orientation.xyz, input.pos.xyz);
    float3 nv = input.pos.xyz + instance.orientation.w * t + cross(instance.orientation.xyz, t);

    nv *= instance.position.w;
    nv += instance.position.xyz;
    pout.pos = mul(float4(nv.xyz, 1.0f), lW); // was gWVP -- testing
    pout.color = input.pos;

    t = 2 * cross(instance.orientation.xyz, input.normal.xyz + input.pos.xyz);
    nv = (input.normal.xyz + input.pos.xyz) + instance.orientation.w * t + cross(instance.orientation.xyz, t);

    pout.debugdata = normalize(mul(float4(nv.xyz, 1.0f), lW));
    pout.id = (uint)3;
    //pout.color = float4(clamp(pout.pos.z / pout.pos.w,0.0f,1.0f),clamp(pout.pos.x,0.0f,1.0f),
    //                    clamp(pout.pos.y,0.0f,1.0f),1.0f);
    //pout.pos.z *= pout.pos.w;
    //pout.color = float4(clamp(pout.pos.z,0.0f,1.0f),clamp(pout.pos.w,0.0f,1.0f),clamp(abs(pout.pos.z-pout.pos.w),0.0f,1.0f),1.0f);
    //pout.color = float4(0.0f,abs(pout.pos.w - pout.pos.z),0.0f,1.0f);

    //pout.pos.xyz += instance.position.xyz;
    //pout.pos = mul(pout.pos, lW);
    //pout.pos = mul(pout.pos, view);
    //pout.pos = mul(float4(input.pos.xyz,1.0f), gWVP);
    //pout.pos += mul(instance.entity_pos, camera);

    return pout;
}

PSShaderOutput BasicPShader(BasicVSOutput input)
{
    //BasicPSOutput pout = (BasicPSOutput)0;
    ////pout.color = input.color;
    //pout.color = input.pos;//float4(,1,1);
    //pow(texColor,1/2.2
    PSShaderOutput output;
    float4 bump = envCubeText1.Sample(environmentCubeSampler, input.color.xyz);
    //bump.rgb = pow(bump.rgb * (1.0f + bump.a * 2), 2); // pow(bump.rgb, 4 * (1.0f + pow(bump.a, 5)));

    //bump.rgb *= 1.0f * bump.a;

    //float4 bump2 = envCubeText1.Sample(environmentCubeSampler, input.color.xyz + float3(1/1280.0f,0.0f,0.0f));
    //float4 base_color = envCubeText.Sample(environmentCubeSampler, input.debugdata.xyz);
    float4 base_color = envCubeText.Sample(environmentCubeSampler, input.color.xyz);
    float4 reflect_color = envCubeText2.Sample(environmentCubeSampler, input.debugdata.xyz * bump.xyz);

    //float4 new_color = 0.5f * base_color + 0.5f * ((1.0f - base_color.a) * base_color + (base_color.a) * reflect_color);
    float4 new_color = (1.0f - base_color.a) * base_color + (base_color.a) * reflect_color;

    float3 disturbed_norm = normalize(-input.debugdata.xyz * bump.xyz);

    float4 light1 = dot(disturbed_norm, normalize((float3(2 * mouseinfo.x, 2 * mouseinfo.y, -1.0f) * 2.0f) - 1.0f));
    //float4 light2 = dot(disturbed_norm, normalize(float3(campos.xy,-campos.z)));
    float4 light2 = dot(disturbed_norm, float3(1.0f, -1.0f, -1.0f));
    //float4 light3 = dot(disturbed_norm, float3(campos.xy, 1.0f));
    float4 light3 = dot(disturbed_norm, float3(-1.0f, -1.0f, -1.0f));

    float4 light4 = light1;

    light1 = pow(light1, 6);
    light4 = pow(light4, 4);
    light3 = pow(light3, 4);
    light2 = pow(light2, 4);

    light1 *= 5 * float4(0.85f, 0.85f, 0.85f, 0.0f);
    light4 *= 3 * float4(0.55f, 0.67f, 0.56f, 0.1f);
    light2 *= float4(0.4f, 0.3f, 0.97f, 0.01f);
    light3 *= float4(0.8f, 0.35f, 0.3f, 0.01f);

    //new_color.a = 0;
    //base_color.a = 0.0f;
    //reflect_color.a

    //bump.a = 0.0f; base_color.a = 0.0f; new_color.a = 0.0f;
    //output.color = 0.3f * new_color * bump.a + 0.3f * base_color + 0.5f * new_color * bump.a * light1 + 0.5f * new_color * bump.a * light2 + 0.5f * bump * new_color * light3;
    //output.color = new_color * bump.a +
    //               //0.3f * base_color  +
    //               reflect_color *  light3  +
    //               0.2f * base_color * bump.a * light2 +
    //               new_color * bump.a * light1 + light1 * new_color;
    //
    //output.color.a *= 0.02f;

    //output.color = output.color * 3;

    //output.color = light1 + light1 * bump.a;
    //output.color = pow(output.color,2);
    //output.color.rgb = bump.rgb * bump.a * reflect_color.rgb;

    //output.color = 0.2f * new_color * light1 * light2 * (1.0f - base_color.a) * bump.a
    //             + 0.2f * base_color * light1 * light3 * (1.0f - base_color.a) * bump.a
    //             + 0.2f * base_color * light4 * (1.0f - base_color.a) * bump.a
    //             + 0.8f * base_color * light4 * (base_color.a) * bump.a;

    //output.color = 0.1f * base_color +
    //               0.1f * base_color * light2 +
    //               0.1f * base_color * light3 +
    //               0.5f * base_color * light4 * bump.a +
    //               0.9f * base_color * light1 * base_color.a * bump.a;

    output.color = 0.1f * base_color + 0.1f * base_color * light2 + 0.1f * base_color * light3 + 0.5f * base_color * light1 * bump.a + 0.6f * base_color * light4 * (1.0f - base_color.a) + 3 * reflect_color * bump * base_color.a;

    output.color.a *= 0.5f * bump.a;

    //output.color = reflect_color + 0.2f * base_color + new_color;
    //output.color = new_color;
    //output.color.a = 0.0f;

    //output.color.a *= bump.a;

    // L = 0.27R + 0.67G + 0.06B;  // Luminance
    //
    //output.color = 0.4f * new_color + 0.3f * new_color * light1 + 0.3f * new_color * light2;
    //output.color = base_color * saturate(dot(input.color.xyz * bump.xyz, float3(0.8, 0.3, 1)));
    //output.color = output.color * (1-saturate(dot(input.color.xyz * bump.xyz, float3(-0.5, -0.3, 1)) * 3));
    //output.color = reflect_color;
    output.id = (uint)3;

    return output;
    //return input.pos;
    //input.color.z += input.color.z * 4;
    //float4 color = input.color;
    //return color;
}