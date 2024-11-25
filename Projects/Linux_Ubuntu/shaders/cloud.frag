#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main()
{
	vec4 color = texture(texSampler, inUV);
	float alpha = (color.x + color.y + color.z) / 3.f;
	outFragColor = vec4(1.f, 0.32f, 0.f, alpha / 3.5f);
}
