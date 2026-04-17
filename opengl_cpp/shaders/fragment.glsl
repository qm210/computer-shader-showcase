#version 430 core

in vec2 uv;
out vec4 fragColor;

layout(binding = 0) uniform usampler2D texState;

const vec3 c = vec3(1, 0, -1);

vec3 cellColor(uint alive) {
    if (alive > 0u) {
        return vec3(0.92, 0.97, 0.88);
    } else {
        return vec3(0.08, 0.09, 0.10);
    }
}

void main() {
    ivec2 sizePx = textureSize(texState, 0);
    ivec2 cell = ivec2(uv * vec2(sizePx));
    cell = clamp(cell, ivec2(0), sizePx - ivec2(1));

    uint alive = texelFetch(texState, cell, 0).r;
    fragColor.rgb = cellColor(alive);
    fragColor.a = 1.;
}
