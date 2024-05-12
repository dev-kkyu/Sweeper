#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {           // �׸� ��ġ (Ȥ�� ���ÿ� ��� ������Ʈ�� �𵨺�ȯ ���)
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
    float accmTime = push.model[0][0];      // �ڵ忡��, �̰��� �����ð��� �־����.
	float t = accmTime - emitTime;          // ���۽ð��� �Ǿ��ٸ�
	vec4 newPosition = vec4(0.f, 0.f, 0.f, 1.f);
    vec4 newColor = vec4(inColor, 1.f);
    if (t < 0.f) {
        newColor.a = 0.f;
    }
    else {
        float newTime = fract(t / lifeTime) * lifeTime;

        newPosition.xy = newTime * emitDir.xy * 2.f;
    }

    // ī�޶��� �ݴ�� ȸ����Ų ��, �׸� ��ġ�� �̵��� �ش�.
    vec3 finalPos = push.model[3].xyz + inverse(mat3(ubo.view)) * (inPosition + newPosition.xyz); 

    gl_Position = ubo.proj * ubo.view * vec4(finalPos, 1.f);
    fragTexCoord = inTexCoord;
    outColor = newColor;
}
