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
	int index = int(time * 20.f) + 3;			// 4��° �̹������� ����  (3 ~ 16 �ε��� ��� : 14��)

	outAlpha = 1.f;
	if (index > 16) {
		outAlpha = (30 - (index - 20)) / 14.f;	// 3 ~ 50 �ε������� ����ϴ� �� (2.35��)
		outAlpha = clamp(outAlpha, 0.f, 1.f);
		index = 16;
	}
	int iu = index % 5;
	int iv = index / 5;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 5.f, (uv[gl_VertexIndex].y + iv) / 4.f);

	vec3 quadPos = quad[gl_VertexIndex];		// �׷��� ��ġ
	quadPos *= 2.5f;							// Ȯ��
	quadPos.y += 0.001f;						// �ٴڿ��� ��¦ ����ֱ�

    vec3 finalPos = push.model[3].xyz + quadPos;	// ������ ��ġ(���� ��ǥ)�� �ű��

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
