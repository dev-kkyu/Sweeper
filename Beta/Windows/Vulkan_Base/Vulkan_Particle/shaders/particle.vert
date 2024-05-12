#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {           // 그릴 위치 (혹은 동시에 띄울 오브젝트의 모델변환 행렬)
	mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 emitDir;
layout(location = 4) in float emitTime;
layout(location = 5) in float lifeTime;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 outColor;

const float c_PI = 3.141592f;

void main() {
    float accmTime = push.model[0][0];      // 코드에서, 이곳에 누적시간을 넣어줬다.
	float t = accmTime - emitTime;          // 시작시간이 되었다면
	vec4 newPosition = vec4(0.f, 0.f, 0.f, 1.f);
    vec4 newColor = vec4(inColor, 1.f);
    if (t < 0.f) {
        newColor.a = 0.f;
    }
    else {
        float newTime = fract(t / lifeTime) * lifeTime;

        newPosition.xy = newTime * emitDir.xy * 2.f;
    }

    // 카메라의 반대로 회전시킨 후, 그릴 위치로 이동해 준다.
    vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * (inPosition + newPosition.xyz); 

    gl_Position = ubo.proj * ubo.view * vec4(finalPos, 1.f);
    fragTexCoord = inTexCoord;
    outColor = newColor;
}
