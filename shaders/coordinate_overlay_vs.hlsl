#include "ConstantBuffers.hlsl"

//
//  Mikel Negugogor (http://github.com/mikelneg)
//

struct VSOut {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VSOut coordinate_overlay_vs(uint vertex_id
                            : SV_VertexID)
{
    VSOut result;

    uint r = exp2(vertex_id); // calculates a box in trianglestrips
    bool x = r & 12;          // 1010 // given 4 vertices
    bool y = r & 10;          // 1100
    //bool x = r & 3;  // 1010
    //bool y = r & 5;  // 1100

    float dx = (2.0f * x) - 1.0f;
    float dy = (2.0f * y) - 1.0f;

    result.position = mouseinfo;
    result.position.x += dx * result.position.w;
    result.position.y += dy * result.position.w;

    result.color = float4(0.4f, result.position.w, 0.4f, 1.0f);

    result.position.w = 1.0f;

    return result;
}

float4 coordinate_overlay_ps(VSOut input)
    : SV_TARGET0
{
    return input.color;
}