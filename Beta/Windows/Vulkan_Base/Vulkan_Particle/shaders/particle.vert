#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {           // �׸� ��ġ (Ȥ�� ���ÿ� ��� ������Ʈ�� �𵨺�ȯ ���)
	mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // ī�޶��� �ݴ�� ȸ����Ų ��, �׸� ��ġ�� �̵��� �ش�.
    vec3 finalPos = inverse(mat3(ubo.view)) * inPosition + push.model[3].xyz; 

    gl_Position = ubo.proj * ubo.view * vec4(finalPos, 1.f);
    fragTexCoord = inTexCoord;
}
