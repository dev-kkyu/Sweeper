#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in float inAlpha;

layout (location = 0) out vec4 outFragColor;

void main()
{
	vec4 color = texture(texSampler, inUV);
	outFragColor = vec4(color.g, color.r, color.b, color.a * inAlpha);
}
