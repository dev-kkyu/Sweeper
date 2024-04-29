#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (set = 2, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 lightSpaceMatrix;
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
layout (location = 5) out vec4 outShadowCoord;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
	outColor = inColor;
	outUV = inUV;

	vec4 FragPos = push.model * vec4(inPos, 1.0);

	gl_Position = ubo.projection * ubo.view * FragPos;
	
	outNormal = normalize(transpose(inverse(mat3(push.model))) * inNormal);

	outLightVec = ubo.lightPos - FragPos.xyz;
	vec3 vPos = vec3(inverse(ubo.view)[3]);
	outViewVec = vPos - FragPos.xyz;

	outShadowCoord = biasMat * ubo.lightSpaceMatrix * FragPos;
}
