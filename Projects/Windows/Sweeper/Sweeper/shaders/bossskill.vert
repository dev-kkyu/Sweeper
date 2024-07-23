#version 450

layout (location = 0) out vec2 outUV;
layout (location = 1) out float outAlpha;

layout (set = 2, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 lightSpace;
	vec3 lightPos;
} ubo;

const vec3 quad[6] = {
	vec3(1.f, 0.f, -1.f),
	vec3(-1.f, 0.f, -1.f),
	vec3(-1.f, 0.f, 1.f),
	vec3(-1.f, 0.f, 1.f),
	vec3(1.f, 0.f, 1.f),
	vec3(1.f, 0.f, -1.f)
};

const vec2 uv[6] = {
	vec2(1.f, 0.f),
	vec2(0.f, 0.f),
	vec2(0.f, 1.f),
	vec2(0.f, 1.f),
	vec2(1.f, 1.f),
	vec2(1.f, 0.f)
};

layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

void main()
{
	float time = push.model[3][3];				// �̰��� �ð� ����
	float framePerSec = 30.f;					// �ʴ� ���� ����

	int index = int(time * framePerSec);		// �ε����� ������ �ö󰣴�

	int iu = index % 4;
	int iv = (index % 16) / 4;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 4.f, (uv[gl_VertexIndex].y + iv) / 4.f);

	outAlpha = 1.f - abs(time - 1.25f) / 1.25f;	// ������ 0 ~ 1 ~ 0 ���� 2.5�ʵ��� ����

	vec3 quadPos = quad[gl_VertexIndex];		// �׷��� ��ġ
	quadPos *= 4.5f;							// Ȯ��
	quadPos.y += 0.05f;							// �ٴڿ��� ��¦ ����ֱ�

    vec3 finalPos = push.model[3].xyz + quadPos;	// ������ ��ġ(���� ��ǥ)�� �ű��

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
