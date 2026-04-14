#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, r8ui) readonly uniform uimage2D srcState;
layout(binding = 1, r8ui) writeonly uniform uimage2D dstState;
uniform ivec2 gridSize;

uint loadCell(ivec2 p) {
    if (p.x < 0 || p.x >= gridSize.x
        || p.y < 0 || p.y >= gridSize.y) {
        return 0u;
    }
    return imageLoad(srcState, p).r;
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    if (p.x >= gridSize.x || p.y >= gridSize.y) {
        return;
    }

    uint neighbors = 0u;
    for (int ix = -1; ix < 2; ix++) {
        for (int iy = -1; iy < 2; iy++) {
            if (ix == 0 && iy == 0) {
                continue;
            }
            neighbors += loadCell(p + ivec2(ix, iy));
        }
    }

    bool wasAlive = loadCell(p) > 0u;
    uint isAlive = 0u;
    if (wasAlive && (neighbors == 2u || neighbors == 3u)) {
        isAlive = 1u;
    }
    if (!wasAlive && neighbors == 3u) {
        isAlive = 1u;
    }

    imageStore(dstState, p, uvec4(isAlive, 0u, 0u, 0u));
}
