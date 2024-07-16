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
	vec3(1.f, 1.f, 0.f),
	vec3(-1.f, 1.f, 0.f),
	vec3(-1.f, -1.f, 0.f),
	vec3(-1.f, -1.f, 0.f),
	vec3(1.f, -1.f, 0.f),
	vec3(1.f, 1.f, 0.f)
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
	float value = fract(time * 1.6f);
	outAlpha = clamp((1.f - value) * 2.f, 0.f, 1.f);
	int index = int(value * 22.f);				// 0 ~ 21
	int iu = index % 5;
	int iv = index / 5;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 5.f, (uv[gl_VertexIndex].y + iv) / 5.f);

	vec3 quadPos = quad[gl_VertexIndex];		// �׷��� ��ġ
	quadPos.y += 1.f;							// �ٴ� ����
	quadPos *= 1.5f;

   vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * quadPos;	// ī�޶� ������ ȸ�� ��, ������ ��ġ(���� ��ǥ)�� �ű��
   finalPos.z -= 0.5f;

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
