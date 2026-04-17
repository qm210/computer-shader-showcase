#version 430 core

out vec2 uv;

// kleiner Trick um weniger Vertizes zu brauchen.
// ist ein übergroßes Dreieck, das aber [-1, +1] abdeckt
const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);

void main() {
    vec2 p = verts[gl_VertexID];
    uv = p * 0.5 + 0.5;
    gl_Position = vec4(p, 0.0, 1.0);
}
