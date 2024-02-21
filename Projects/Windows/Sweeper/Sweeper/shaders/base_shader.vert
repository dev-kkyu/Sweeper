#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPos;

void main() {
    vec4 position = push.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * position;

    outNormal = mat3(inverse(transpose(push.model))) * inNormal;
    fragTexCoord = inTexCoord;
    fragPos = vec3(position);
}
