#version 430 core

// jede Work Goup soll 16x16 Zellen bearbeiten (~ 256 Threads)
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, r8ui) readonly uniform uimage2D prevState;
layout(binding = 1, r8ui) writeonly uniform uimage2D newState;
uniform ivec2 gridSize;

uint loadCell(ivec2 p) {
    if (p.x < 0 || p.x >= gridSize.x
        || p.y < 0 || p.y >= gridSize.y) {
        return 0u;
    }
    return imageLoad(prevState, p).r;
}

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    if (coord.x >= gridSize.x || coord.y >= gridSize.y) {
        return;
    }

    uint neighbors = 0u;
    for (int ix = -1; ix < 2; ix++) {
        for (int iy = -1; iy < 2; iy++) {
            if (ix == 0 && iy == 0) {
                continue;
            }
            neighbors += loadCell(coord + ivec2(ix, iy));
        }
    }

    bool wasAlive = loadCell(coord) > 0u;
    uint isAlive = 0u;
    if (wasAlive && (neighbors == 2u || neighbors == 3u)) {
        isAlive = 1u;
    }
    if (!wasAlive && neighbors == 3u) {
        isAlive = 1u;
    }

    // wir schreiben nur einen eindimensionalen Vektor,
    // aber imageStore() erwartet trotzdem uvec4, daher:
    imageStore(newState, coord, uvec4(isAlive, 0u, 0u, 0u));
}
