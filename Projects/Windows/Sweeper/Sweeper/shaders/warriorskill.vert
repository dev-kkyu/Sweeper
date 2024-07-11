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
	float time = push.model[3][3];				// 이곳에 시간 저장
	int index = int(fract(time * 2.f) * 16.f);	// 0 ~ 15
	int iu = index % 4;
	int iv = index / 4;

	outUV = (uv[gl_VertexIndex] + vec2(iu, iv)) / 4.f;

	vec3 quadPos = quad[gl_VertexIndex];		// 그려질 위치
	quadPos.y += 1.f;							// 바닥 위로
	quadPos.x *= 1.5f;							// 좌우 크기 확대
	
	float offset = clamp(time, 0.f, 1.f);		// 1초까지 0 ~ 1
	quadPos *= 0.7f + vec3(offset * 1.3f);		// 0.7 ~ 2배

    vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * quadPos;	// 카메라 각도로 회전 후, 정해진 위치(월드 좌표)로 옮기기
	finalPos.z -= 1.f;							// 화면 아래쪽으로 내려주기

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);

	outAlpha = 1.f - offset;
}
