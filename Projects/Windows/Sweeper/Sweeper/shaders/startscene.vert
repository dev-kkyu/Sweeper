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

// ¹Ì·¡¸¦ À§ÇØ ³öµÐ´Ù
layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

void main()
{
	outUV = uv[gl_VertexIndex];

	gl_Position = vec4(quad[gl_VertexIndex], 1.f);
}
