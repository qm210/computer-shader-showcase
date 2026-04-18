#version 430 core

in vec2 uv;
out vec4 fragColor;

struct Cell {
    uint alive;
    float hue;
    float age;
    float ageMax;
};

layout(std430, binding = 1) readonly buffer StateBuffer {
    Cell state[];
};
uniform ivec2 gridSize;

vec3 hsv2rgb(vec3 c)
{
    // Erwartet HSV-Vector jeweils mit Werten 0..1 (modulo für Hue)
    c.x *= 6.283;
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 cellColor(Cell cell) {
    vec3 bg = vec3(0.08, 0.09, 0.10);
    vec3 col = hsv2rgb(vec3(cell.hue, 1., 1.));
    if (cell.alive > 0u) {
        const float minOpacity = 0.1;
        float ageRatio = clamp(cell.age / cell.ageMax, 0., 1.);
        float opacity = cell.ageMax > 0. ? mix(1., minOpacity, ageRatio) : 1.;
        return mix(bg, col, opacity);
    } else {
        return bg;
    }
}

void main() {
    ivec2 coord = ivec2(uv * vec2(gridSize));
    coord = clamp(coord, ivec2(0), gridSize - ivec2(1));
    int index = coord.y * gridSize.x + coord.x;
    fragColor.rgb = cellColor(state[index]);
    fragColor.a = 1.;
}
