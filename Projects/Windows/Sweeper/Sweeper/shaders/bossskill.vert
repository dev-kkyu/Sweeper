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
	float framePerSec = 30.f;					// 초당 사진 개수

	int index = int(time * framePerSec);		// 인덱스는 무한히 올라간다

	int iu = index % 4;
	int iv = (index % 16) / 4;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 4.f, (uv[gl_VertexIndex].y + iv) / 4.f);

	outAlpha = 1.f - abs(time - 1.25f) / 1.25f;	// 투명도를 0 ~ 1 ~ 0 으로 2.5초동안 동작

	vec3 quadPos = quad[gl_VertexIndex];		// 그려질 위치
	quadPos *= 4.5f;							// 확대
	quadPos.y += 0.05f;							// 바닥에서 살짝 띄워주기

    vec3 finalPos = push.model[3].xyz + quadPos;	// 정해진 위치(월드 좌표)로 옮기기

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
