/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

cbuffer root_constants : register(b0, space1) {
  float4 viewport; // top_left_x, top_left_y, width, height
};

float screen_ratio() { return viewport.z / viewport.w; }

struct VertexInputType {
  float4 position : VERTEX_POSITION;
  float4 normal : VERTEX_NORMAL;
  uint4 bone_ids : VERTEX_BONE_IDS;
  float4 bone_weights : VERTEX_BONE_WEIGHTS;
  float2 uv : VERTEX_UV;
};

struct InstanceInputType {
  uint4 info : INSTANCE_INFO;
  // <bone_offset, material_id, entity_id, mesh_id>
};

struct OutputType {
  float4 pos : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
  uint id : OBJECT_ID;
};

struct ps_output {
  float4 color : SV_Target0;
  uint id : SV_Target1;
};

cbuffer per_frame : register(b0) { 
    float4x4 view; 
    float4x4 proj;
};

cbuffer per_frame_also : register(b0, space2) { uint bone_count; };

struct bone {
  float4x4 t;
};

StructuredBuffer<bone> bones_ : register(t1, space2);

Texture2D color_texture_ : register(t0);

SamplerState sampler_ : register(s0) {
  Filter = MIN_MAG_MIP_POINT;
  AddressU = Clamp;
  AddressV = Clamp;
  AddressW = Clamp;
};

OutputType vs_main(in uint instance_id
                   : SV_InstanceID, VertexInputType input_,
                     InstanceInputType instance_) {
  OutputType output_ = (OutputType)0;

  static const float4x4 swap_z_y = {1.0f, 0.0f, 0.0f, 0.0f, 
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f};

  float4 pos = float4(0.0f,0.0f,0.0f,1.0f);
  float4 norm = float4(0.0f,0.0f,0.0f,0.0f);   
  
  for (uint i = 0; i < bone_count; ++i) { 
    pos += mul(input_.position, bones_[instance_.info.x + input_.bone_ids[i] ].t) * input_.bone_weights[i];
    norm += mul(input_.normal,  bones_[instance_.info.x + input_.bone_ids[i] ].t) * input_.bone_weights[i];
  }

  pos.w = 1.0f;
  norm.w = 0.0f;

  static const float4x4 trans = mul(view,mul(proj,swap_z_y));
  
  output_.pos = mul(trans,pos);      
  output_.pos.y *= screen_ratio();  
  
 
  // logarithmic depth buffer 
  
  // float coef = 2.0 / log2(far_z + 1.0f);
  // output_.pos.z = log(max(0.00001f,output_.pos.w + 1)) / log(200.0f + 1) * output_.pos.w; //log2(max(0.0001f, 1.0 + output_.pos.w)) * coef - 1.0;            

  output_.color = normalize(mul(norm,view));

  output_.color += 1.0f; 
  output_.color *= 0.5f;

  output_.uv = input_.uv;
  output_.id = instance_.info.z;
  return output_;
}

ps_output ps_main(OutputType input) {
  ps_output out_;
  
  float4 samp = color_texture_.Sample(sampler_, input.uv);  

  out_.color.rgb = (1.0f-samp.a) * pow(dot(input.color.rgb,float3(-1,-1,1)),2) + input.color.rgb * (samp.a);  
  //out_.color.rgb = input.color.rgb * (1.0f - samp.a) + (samp.a) * pow(dot(input.color.rgb + samp.xyz,float3(-1.0f,-1.0f,1.0f)),7);    
  //out_.color.rgb = pow(dot(input.color.rgb,float3(-1.0f,-1.0f,1.0f)),2) * samp * 0.4f + samp * 0.3f + input.color.rgb * 0.3f;
  //out_.color.rgb = samp * 0.2f + pow(dot(input.color.rgb + (samp.xyz * samp.a),float3(-1.0f,-1.0f,1.0f)),6) * 0.3f + 0.6f * input.color.rgb;
  //out_.color *= float4(0.4,0.7,0.9,1.0);
  out_.color.a = 1.0f;
  out_.id = input.id;
  return out_;
}