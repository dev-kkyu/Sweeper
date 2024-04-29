#version 450

layout (location = 0) in vec3 inPos;
// layout (location = 1) in vec3 inNormal;
// layout (location = 2) in vec2 inUV;
// layout (location = 3) in vec3 inColor;
layout (location = 4) in vec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

layout (set = 2, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	mat4 lightSpaceMatrix;
	vec3 lightPos;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout(std430, set = 3, binding = 0) readonly buffer JointMatrices {
	mat4 jointMatrices[];
};

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	// 현재 vertex pos 의 가중치와 joint 인덱스로부터 스키닝 처리된 행렬을 계산한다.
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];

	gl_Position = ubo.lightSpaceMatrix * push.model * skinMat * vec4(inPos, 1.0);
}
