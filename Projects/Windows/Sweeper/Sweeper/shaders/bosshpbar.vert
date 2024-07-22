#version 450

layout (location = 0) out vec3 outColor;

const vec3 quad[6] = {
	vec3(1.f, 1.f, 0.f),
	vec3(-1.f, 1.f, 0.f),
	vec3(-1.f, -1.f, 0.f),
	vec3(-1.f, -1.f, 0.f),
	vec3(1.f, -1.f, 0.f),
	vec3(1.f, 1.f, 0.f)
};

const vec3 color[2] = {
	vec3(0.5f, 0.5f, 0.5f),
	vec3(1.f, 0.f, 0.f)
};

layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

void main()
{
	float offset = push.model[3][3];	// �̰��� ü�� �ۼ�Ʈ 0 ~ 1�� �־����

	vec3 quadPos = quad[gl_VertexIndex % 6];
	int flag = gl_VertexIndex / 6;		// 0(ȸ�����) or 1(����ü��)

	// ���� �簢���� ��� ���� ü�¿� ���� ������ ��ġ�� ����
	quadPos.x = clamp(quadPos.x, quadPos.x, (1 - flag) * quadPos.x + flag * (offset * 2.f - 1.f));
	quadPos.x *= 0.75f;
	quadPos.y *= 0.02f;
	quadPos.y += 0.85f;

	outColor = color[flag];
	gl_Position = vec4(quadPos, 1.f);
}
