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
	float time = push.model[3][3];				// 이곳에 시간 저장

	int index = 0;								// 첫번째 사진만 사용한다

	int iu = index % 4;
	int iv = (index % 16) / 4;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 4.f, (uv[gl_VertexIndex].y + iv) / 4.f);

	if (time < 1.1f)
		outAlpha = time / 1.1f;
	else
		outAlpha = 1.f - ((time - 1.1f) / 1.1f) * 5.f;	// 1.1초 이후에 5배 빨리 사라진다 (1 ~ 0), 총길이 1.1 + 0.22

	vec3 quadPos = quad[gl_VertexIndex];		// 그려질 위치
	quadPos *= 4.5f;							// 확대
	quadPos.y += 0.05f;							// 바닥에서 살짝 띄워주기

    vec3 finalPos = push.model[3].xyz + quadPos;	// 정해진 위치(월드 좌표)로 옮기기

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
