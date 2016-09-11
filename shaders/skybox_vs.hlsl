/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

cbuffer per_frame : register(b0)
{
    float4x4 view;
};

TextureCube skybox_texture : register(t0);
//Texture2D color_pallete_texture : register( t1 );

SamplerState skybox_sampler : register(s0);
//SamplerState pallete_sampler : register( s1 );

struct vs_out {
    float4 position : SV_Position;
    float3 uvw : COLOR0;
};

struct ps_out {
    float4 color : SV_Target0;
    //float4 normal : SV_Target1;
    uint id : SV_Target1;
};

vs_out vs_main(uint vertex_id
               : SV_VertexID)
{
    // Generates a cube with 14 counter-clockwise oriented triangles
    // that corner-originates at (0,0,0) and extends into positive XYZ;
    // the cube is then re-positioned with center at (0,0,0) and
    // transformed by the camera-projection matrix

    vs_out result;

    uint r = exp2(vertex_id); // r = 2^vertex_id
    bool x = r & 3612;        // 0000111000011100
    bool y = r & 10362;       // 0010100001111010
    bool z = r & 687;         // 0000001010101111
                              // So the x,y,z coordinate for vertex_0 is
                              // (0,0,1) (reading down the 0th column),
                              // vertex_1 is (0,1,1), vertex_2 is (1,0,1), etc.

    // use the view matrix _without_ translation

    //static float4x4 rot_only = { view._11, view._12, view._13, view._14,
    //                             view._21, view._22, view._23, view._24,
    //                             view._31, view._32, view._33, view._34,
    //                             0,0,0, view._44 };

    //
    // Extract the "facing" direction vector of the camera
    //result.camera_direction = float4(view._13,view._23,view._33,0.0f);

    // Center at (0,0,0)
    result.uvw = float3(x, y, z) * 2.0f - 1.0f;
    //result.uvw = float3(vertex_id,vertex_id,vertex_id);

    // ".xyzz" sets w == z so that z/w == 1, forcing the pixel to the far plane
    // in the depth buffer

    //result.position = mul(float4(result.uvw, 1.0f), rot_only).xyzz;
    result.position = mul(result.uvw, (float3x3)view).xyzz;

    //result.uvw = float3(view._11, view._12, 1.0f / vertex_id);
    //result.position = float4(result.uvw, 1.0f).xyzz;

    return result;
}

ps_out ps_main(vs_out input)
{
    // Unused code: displaces each uvw by some function of its distance to the
    // camera's facing vector, creating a "lens" like effect.. sort of neat,
    // probably faster/nicer to displace the final composed scene with a function
    // of screen coordinates (in some other shader)
    //
    //    float3 uvw = normalize(input.uvw.xyz);
    //    float3 tvec = (input.camera_direction.xyz / dot(uvw, input.camera_direction.xyz)) - uvw;
    //    float dist = length(tvec);
    //   //
    //   // float3 mvec = dot(uvw,mouseinfo.xyz);
    //
    //    //dist = (cos(dist*3.14159)/20)+0.05;      // some interesting "lens" functions
    //
    //
    //    dist = 0.3f + sin(dist * 30 * length(mouseinfo.xy)) * cos(dist) * 2 * mouseinfo.w;
    //
    //     //dist = 0.3f + sin(dist * 10 * mvec) * cos(dist) * 2 * mvec; // wormhole
    //
    //
    //    //dist = 1-(dist*dist*4);
    //    //dist = cos(4*dist)*sin(dist);
    //    //dist = tan(4*mvec)*sin(length(mvec));
    //    //dist = 1-(dist*dist);
    //    //dist = (dist/(14*log(dist)))+1;
    //    //dist = (dist/50*log(dist))+1;
    //    //dist = (dist/(4*log(dist)))+1; // neat lens
    //    //dist = -dist*dist+1;
    //    //dist = 0.2*(1/(dist*dist*dist*dist*dist*dist+0.2));
    //    //dist = cos((1/dist)*(3.14159/10));  // blackhole!
    //   // dist = 1-tan(dist*dist);
    //
    //    uvw.xyz -= tvec * dist;
    //    return SkyBoxTexture.Sample(SkyBoxSampler, uvw.xyz);
    //float4 color = SkyBoxTexture.Sample(SkyBoxSampler, input.uvw.xyz);
    //return input.position * 0.2f;
    //return float4(input.uvw,1.0f);
    ps_out result;
    result.color = skybox_texture.Sample(skybox_sampler, input.uvw.xyz);
    result.id = 0;
    //result.color = color_pallete_texture.Sample(skybox_sampler, input.uvw.xy);
    //result.color = float4(input.uvw.xyz, 1.0f);
    //result.normal = float4(input.uvw.xyz, 1.0f);
    return result;
}