#version 450

layout (location = 0) out vec2 outUV;

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
	float time = push.model[3][3];					// �̰��� �ð� ����
	int index = int(fract(time * 1.5f) * 35.f);		// 0 ~ 34
	int iu = index % 5;
	int iv = index / 5;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 5.f, (uv[gl_VertexIndex].y + iv) / 7.f);

	vec3 quadPos = quad[gl_VertexIndex];		// �׷��� ��ġ
	quadPos.y += 0.1f;							// �̹����� ���߾� �ٴ� ����
	quadPos *= 0.25f;							// ������ �۰�

   vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * quadPos;	// ī�޶� ������ ȸ�� ��, ������ ��ġ(���� ��ǥ)�� �ű��

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
