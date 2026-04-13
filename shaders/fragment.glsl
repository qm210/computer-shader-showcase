#version 430 core

in vec2 uv;
out vec4 outColor;

layout(binding = 0) uniform usampler2D uStateTex;

const vec3 c = vec3(1, 0, -1);

const vec3 deadColor = vec3(0.08, 0.09, 0.10);
const vec3 liveColor = vec3(0.92, 0.97, 0.88);

void main() {
    ivec2 sizePx = textureSize(uStateTex, 0);
    ivec2 cell = ivec2(uv * vec2(sizePx));
    cell = clamp(cell, ivec2(0), sizePx - ivec2(1));

    uint alive = texelFetch(uStateTex, cell, 0).r;

    vec3 col = alive != 0u ? liveColor : deadColor;

    outColor = vec4(col, 1.0);
}
