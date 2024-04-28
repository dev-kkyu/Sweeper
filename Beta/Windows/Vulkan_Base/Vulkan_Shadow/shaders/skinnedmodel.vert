#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

layout (set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	vec3 lightPos;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout(std430, set = 2, binding = 0) readonly buffer JointMatrices {
	mat4 jointMatrices[];
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main() 
{
	outColor = inColor;
	outUV = inUV;

	// ���� vertex pos �� ����ġ�� joint �ε����κ��� ��Ű�� ó���� ����� ����Ѵ�.
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];

	vec4 FragPos = push.model * skinMat * vec4(inPos, 1.0);

	gl_Position = ubo.projection * ubo.view * FragPos;
	
	outNormal = normalize(transpose(inverse(mat3(push.model * skinMat))) * inNormal);

	outLightVec = ubo.lightPos - FragPos.xyz;
	vec3 vPos = vec3(inverse(ubo.view)[3]);
	outViewVec = vPos - FragPos.xyz;
}
