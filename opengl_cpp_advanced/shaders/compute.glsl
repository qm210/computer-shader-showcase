#version 430 core

// jede Work Goup soll 16x16 Zellen bearbeiten (~ 256 Threads)
layout(local_size_x = 16, local_size_y = 16) in;

struct Cell {
    uint alive;
    float hue;
    float age;
    float ageMax;
};

layout(std430, binding = 0) readonly buffer PrevBuffer {
    Cell prevState[];
};
layout(std430, binding = 1) writeonly buffer NewBuffer {
    Cell newState[];
};
uniform ivec2 gridSize;
uniform float time;
uniform float timeDelta;
uniform float ageMaxSec;

int index(ivec2 p) {
    return p.y * gridSize.x + p.x;
}

Cell dead() {
    return Cell(0u, 0., 0., 0.);
}

Cell load(ivec2 p) {
    if (p.x < 0 || p.x >= gridSize.x
    || p.y < 0 || p.y >= gridSize.y) {
        return dead();
    }
    return prevState[index(p)];
}

float pseudorandomHash(vec2 p, float seed) {
    p = vec2(dot(p, vec2(127.1, 311.7)) + seed * 17.3, seed * 23.7);
    float s = fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
    return sin(s);
}

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    if (coord.x >= gridSize.x || coord.y >= gridSize.y) {
        return;
    }

    uint neighbors = 0u;
    float sumHue = 0.;
    for (int ix = -1; ix < 2; ix++) {
        for (int iy = -1; iy < 2; iy++) {
            if (ix == 0 && iy == 0) {
                continue;
            }
            Cell neighbor = load(coord + ivec2(ix, iy));
            neighbors += neighbor.alive;
            if (neighbor.alive > 0u) {
                sumHue += neighbor.hue;
            }
        }
    }
    float avgHue = neighbors > 0u
        ? sumHue / float(neighbors)
        : 0.;

    Cell cell = load(coord);

    bool wasAlive = cell.alive > 0u;
    Cell cellNow = dead();

    // S23: Survival, mit Alter
    if (wasAlive && (neighbors == 2u || neighbors == 3u)) {
        float age = cell.age + timeDelta;
        cellNow = Cell(1u, avgHue, age, ageMaxSec);
        if (age >= cell.ageMax && cell.ageMax > 0.) {
            cellNow = dead();
        }
    }
    // B3: Birth
    if (!wasAlive && neighbors == 3u) {
        // Farbe darf ein bisschen variieren, weil, warum nicht?
        float hue = avgHue + 0.0 * pseudorandomHash(vec2(coord), time);
        cellNow = Cell(1u, hue, 0., ageMaxSec);
    }

    newState[index(coord)] = cellNow;
}
