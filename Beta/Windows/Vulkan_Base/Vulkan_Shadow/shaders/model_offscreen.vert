#version 450

layout (location = 0) in vec3 inPos;
// layout (location = 1) in vec3 inNormal;
// layout (location = 2) in vec2 inUV;
// layout (location = 3) in vec3 inColor;

layout (set = 2, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 lightSpaceMatrix;
	vec3 lightPos;
} ubo;

layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position = ubo.lightSpaceMatrix * push.model * vec4(inPos, 1.0);
}
