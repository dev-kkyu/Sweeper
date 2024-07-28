#version 450

layout (location = 0) out vec2 outUV;
layout (location = 1) out float outAlpha;

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
	outUV = uv[gl_VertexIndex];

	outAlpha = push.model[3][3];	// 이곳에 넣어둔다. 따로 설정 안하면 1.f

	gl_Position = vec4(quad[gl_VertexIndex], 1.f);
}
