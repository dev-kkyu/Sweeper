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
	int index = int(time * 20.f) + 3;			// 4번째 이미지부터 시작  (3 ~ 16 인덱스 사용 : 14개)

	outAlpha = 1.f;
	if (index > 16) {
		outAlpha = (30 - (index - 20)) / 14.f;	// 3 ~ 50 인덱스까지 사용하는 것 (2.35초)
		outAlpha = clamp(outAlpha, 0.f, 1.f);
		index = 16;
	}
	int iu = index % 5;
	int iv = index / 5;

	outUV = vec2((uv[gl_VertexIndex].x + iu) / 5.f, (uv[gl_VertexIndex].y + iv) / 4.f);

	vec3 quadPos = quad[gl_VertexIndex];		// 그려질 위치
	quadPos *= 2.5f;							// 확대
	quadPos.y += 0.001f;						// 바닥에서 살짝 띄워주기

    vec3 finalPos = push.model[3].xyz + quadPos;	// 정해진 위치(월드 좌표)로 옮기기

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
