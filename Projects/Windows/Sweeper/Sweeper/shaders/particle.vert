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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float emitTime;
layout(location = 2) in float lifeTime;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

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

const float c_PI = 3.141592f;

void main() {
    float accmTime = push.model[0][0];      // 코드에서, 이곳에 누적시간을 넣어줬다.
	float t = accmTime - emitTime;          // 시작시간이 되었다면
	vec4 newPosition = vec4(0.f, 0.f, 0.f, 1.f);
    vec4 newColor = vec4(1.f, 1.f, 1.f, 1.f);
    if (t < 0.f) {
        newColor.a = 0.f;
    }
    else {
        float newTime = fract(t / lifeTime) * lifeTime;
        newPosition.y = newTime * 1.f;	// lifetime의 y좌표로 가준다
		newColor.a = 1.f - (newTime / lifeTime);
		newColor.a = clamp(newColor.a * 5.f, 0.f, 1.f);
    }

    outColor = newColor;

    outUV = uv[gl_VertexIndex % 6];

    // 카메라의 반대로 회전시킨 후, 그릴 위치로 이동해 준다.
	vec3 quadPos = quad[gl_VertexIndex % 6] * 0.2f;		// 사이즈 조정
	// 직교행렬이기 때문에 inverse와 transpose가 같다. transpose가 더 빠르다
    vec3 finalPos = push.model[3].xyz + newPosition.xyz + inPosition + transpose(mat3(ubo.view)) * quadPos;

    gl_Position = ubo.projection * ubo.view * vec4(finalPos, 1.f);
}
