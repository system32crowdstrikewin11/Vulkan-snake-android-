#version 450

// Data sent from the C++ code for each object
layout(push_constant) uniform constants {
    vec2 pos;
    vec2 size;
    vec3 color;
} PushConstants;

// Pass the color to the fragment shader
layout(location = 0) out vec3 fragColor;

// Vertices for a unit square
vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2( 0.5,  0.5),
    vec2(-0.5, -0.5),
    vec2( 0.5,  0.5),
    vec2(-0.5,  0.5)
);

void main() {
    // Scale and position the square
    gl_Position = vec4(positions[gl_VertexIndex] * PushConstants.size + PushConstants.pos, 0.0, 1.0);
    fragColor = PushConstants.color;
}
