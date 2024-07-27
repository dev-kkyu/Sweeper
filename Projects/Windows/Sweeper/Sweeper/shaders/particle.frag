#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outFragColor;

void main() {
    outFragColor = texture(texSampler, inUV) * inColor;
}
