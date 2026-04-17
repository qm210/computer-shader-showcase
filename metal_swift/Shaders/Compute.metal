#include <metal_stdlib>
using namespace metal;

struct GridSize {
    uint width;
    uint height;
};

kernel void stepLife(texture2d<uint, access::read>  srcState [[texture(0)]],
                     texture2d<uint, access::write> dstState [[texture(1)]],
                     constant GridSize& grid [[buffer(0)]],
                     uint2 gid [[thread_position_in_grid]]
) {
    if (gid.x >= grid.width || gid.y >= grid.height)
        return;

    uint alive = srcState.read(gid).r;
    uint neighbors = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = int(gid.x) + dx;
            int ny = int(gid.y) + dy;
            if (nx < 0 || ny < 0 || nx >= int(grid.width) || ny >= int(grid.height)) continue;
            neighbors += srcState.read(uint2(nx, ny)).r;
        }
    }

    uint nextAlive = 0;
    if (alive == 1) {
        nextAlive = (neighbors == 2 || neighbors == 3) ? 1 : 0;
    } else {
        nextAlive = (neighbors == 3) ? 1 : 0;
    }

    dstState.write(uint4(nextAlive, 0, 0, 0), gid);
}
