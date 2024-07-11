#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main()
{
	vec4 color = texture(texSampler, inUV);
	outFragColor = vec4(vec3(0.35f, 0.2f, 0.05f) * color.rgb, color.a * 2.f);
}
