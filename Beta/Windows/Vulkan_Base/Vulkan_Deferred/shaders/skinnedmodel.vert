#version 450

layout (set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 projection;
	mat4 lightSpace;
	vec4 lightPos;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout(std430, set = 3, binding = 0) readonly buffer JointMatrices {
	mat4 jointMatrices[];
};

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

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

	// 현재 vertex pos 의 가중치와 joint 인덱스로부터 스키닝 처리된 행렬을 계산한다.
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];

	gl_Position = ubo.projection * ubo.view * push.model * skinMat * vec4(inPos.xyz, 1.0);
	
	outNormal = normalize(transpose(inverse(mat3(ubo.view * push.model * skinMat))) * inNormal);

	vec4 pos = ubo.view * vec4(inPos, 1.0);
	vec3 lPos = mat3(ubo.view) * ubo.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;

	outShadowCoord = ( biasMat * ubo.lightSpace * push.model ) * vec4(inPos, 1.0);
}