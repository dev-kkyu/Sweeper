#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;
layout (set = 2, binding = 0) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

#define ambient 0.5

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

void main() {
    vec4 Color = texture(texSampler, fragTexCoord);
	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	outColor = Color * shadow;
}
