#version 450

layout (set = 2, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 lightSpace;
	vec3 lightPos;
} ubo;

layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

const vec3 cube[48] = {
	// Top
	vec3(-0.5f, 0.5f, -0.5f), vec3(-0.5f, 0.5f, 0.5f),
	vec3(-0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f),
	vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, -0.5f),
	vec3(0.5f, 0.5f, -0.5f), vec3(-0.5f, 0.5f, -0.5f),
	// Bottom
	vec3(-0.5f, -0.5f, -0.5f), vec3(-0.5f, -0.5f, 0.5f),
	vec3(-0.5f, -0.5f, 0.5f), vec3(0.5f, -0.5f, 0.5f),
	vec3(0.5f, -0.5f, 0.5f), vec3(0.5f, -0.5f, -0.5f),
	vec3(0.5f, -0.5f, -0.5f), vec3(-0.5f, -0.5f, -0.5f),
	// Front
	vec3(-0.5f, 0.5f, 0.5f), vec3(-0.5f, -0.5f, 0.5f),
	vec3(-0.5f, -0.5f, 0.5f), vec3(0.5f, -0.5f, 0.5f),
	vec3(0.5f, -0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f),
	vec3(0.5f, 0.5f, 0.5f), vec3(-0.5f, 0.5f, 0.5f),
	// Back
	vec3(-0.5f, 0.5f, -0.5f), vec3(-0.5f, -0.5f, -0.5f),
	vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, -0.5f, -0.5f),
	vec3(0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, -0.5f),
	vec3(0.5f, 0.5f, -0.5f), vec3(-0.5f, 0.5f, -0.5f),
	// Left
	vec3(-0.5f, 0.5f, -0.5f), vec3(-0.5f, -0.5f, -0.5f),
	vec3(-0.5f, -0.5f, -0.5f), vec3(-0.5f, -0.5f, 0.5f),
	vec3(-0.5f, -0.5f, 0.5f), vec3(-0.5f, 0.5f, 0.5f),
	vec3(-0.5f, 0.5f, 0.5f), vec3(-0.5f, 0.5f, -0.5f),
	// Right
	vec3(0.5f, 0.5f, -0.5f), vec3(0.5f, -0.5f, -0.5f),
	vec3(0.5f, -0.5f, -0.5f), vec3(0.5f, -0.5f, 0.5f),
	vec3(0.5f, -0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f),
	vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, -0.5f)
};

void main()
{
	gl_Position = ubo.projection * ubo.view * push.model * vec4(cube[gl_VertexIndex], 1.0);
}
