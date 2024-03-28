#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {           // 그릴 위치 (혹은 동시에 띄울 오브젝트의 모델변환 행렬)
	mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // 카메라의 반대로 회전시킨 후, 그릴 위치로 이동해 준다.
    vec3 finalPos = inverse(mat3(ubo.view)) * inPosition + push.model[3].xyz; 

    gl_Position = ubo.proj * ubo.view * vec4(finalPos, 1.f);
    fragTexCoord = inTexCoord;
}
