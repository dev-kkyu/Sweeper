#version 450

layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 2, binding = 0) uniform sampler2D shadowMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;
layout (location = 5) in vec4 inShadowCoord;

layout (location = 0) out vec4 outFragColor;

#define ambient 0.1

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

void main() 
{
	vec4 color = texture(samplerColorMap, inUV) * vec4(inColor, 1.0);

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), ambient) * inColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	outFragColor = vec4((diffuse * color.rgb + specular) * shadow, 1.0);		
}