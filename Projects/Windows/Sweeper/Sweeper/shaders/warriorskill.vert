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
	float time = fract(push.model[3][3] * 2.f) * 16.f;	// 0 ~ 15.99
	int iu = int(time) % 4;
	int iv = int(time) / 4;

	outUV = (uv[gl_VertexIndex] + vec2(iu, iv)) * 0.25f;

	vec3 movePos = quad[gl_VertexIndex];
	movePos.y += 1.f;
    vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * movePos;

	gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.0);
}
