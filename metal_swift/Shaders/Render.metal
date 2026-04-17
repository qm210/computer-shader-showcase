#include <metal_stdlib>
using namespace metal;

struct VSOut {
    float4 position [[position]];
    float2 uv;
};

vertex VSOut fullscreenVertex(uint vid [[vertex_id]]) {
    float2 pos[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };
    VSOut out;
    out.position = float4(pos[vid], 0.0, 1.0);
    out.uv = pos[vid] * 0.5 + 0.5;
    return out;
}

fragment float4 lifeFragment(VSOut in [[stage_in]],
                             texture2d<uint, access::sample> stateTex [[texture(0)]]
) {
    constexpr sampler s(coord::normalized, address::clamp_to_edge, filter::nearest);
    uint alive = stateTex.sample(s, in.uv).r;
    float c = alive > 0 ? 1.0 : 0.0;
    return float4(c, c, c, 1.0);
}
