cbuffer PerFrame : register(b0)
{
    float4x4 gWVP;
    float4x4 camera;
    float4x4 view;
    float4x4 lW;
    float4 rotationQuat;
    float4 campos;
    float4 mouseinfo;
};

cbuffer PerScene : register(b1)
{
    float4 screen_viewport;
}

float screen_ratio()
{
    return screen_viewport.x / screen_viewport.y;
}