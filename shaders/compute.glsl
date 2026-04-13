#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, r8ui) readonly uniform uimage2D srcState;
layout(binding = 1, r8ui) writeonly uniform uimage2D dstState;
uniform ivec2 uGridSize;

ivec2 wrapCell(ivec2 p) {
    return ivec2(
        (p.x + uGridSize.x) % uGridSize.x,
        (p.y + uGridSize.y) % uGridSize.y
    );
}

uint loadCell(ivec2 p) {
    return imageLoad(srcState, wrapCell(p)).r;
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    if (p.x >= uGridSize.x || p.y >= uGridSize.y) {
        return;
    }

    uint n = 0u;
    n += loadCell(p + ivec2(-1, -1));
    n += loadCell(p + ivec2( 0, -1));
    n += loadCell(p + ivec2( 1, -1));
    n += loadCell(p + ivec2(-1,  0));
    n += loadCell(p + ivec2( 1,  0));
    n += loadCell(p + ivec2(-1,  1));
    n += loadCell(p + ivec2( 0,  1));
    n += loadCell(p + ivec2( 1,  1));

    uint self = loadCell(p);

    uint next = ((self == 1u && (n == 2u || n == 3u)) || (self == 0u && n == 3u)) ? 1u : 0u;
    imageStore(dstState, p, uvec4(next, 0u, 0u, 0u));
}
