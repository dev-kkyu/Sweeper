#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	vec3 lightPos;
} ubo;

layout (push_constant) uniform PushConstants {
	mat4 model;
} push;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main()
{
	outColor = inColor;
	outUV = inUV;

	gl_Position = ubo.projection * ubo.view * push.model * vec4(inPos, 1.0);
	
	outNormal = normalize(transpose(inverse(mat3(ubo.view * push.model))) * inNormal);

	vec4 pos = ubo.view * vec4(inPos, 1.0);
	vec3 lPos = mat3(ubo.view) * ubo.lightPos;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;
}
