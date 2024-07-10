#version 450

layout (location = 0) out vec2 outUV;

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
	float xOffset = push.model[3].x / 55.f;

	vec2 tempUv = uv[gl_VertexIndex];
	tempUv.x /= 2.f;								// 텍스처를 절반으로 나눈다
	tempUv.x += fract(push.model[3].w - xOffset);	// w에 time을 넣어 두었다.
	outUV = tempUv;

	gl_Position = vec4(quad[gl_VertexIndex], 1.0);
}
