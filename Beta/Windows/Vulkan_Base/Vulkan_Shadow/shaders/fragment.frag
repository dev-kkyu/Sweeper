#version 450

layout (set = 0, binding = 0) uniform sampler2D shadowMap;
layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;
layout (location = 5) in vec4 inShadowCoord;

layout (location = 0) out vec4 outFragColor;

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;		// 필터링 범위
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx * x, dy * y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main()
{
//	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
	float shadow = filterPCF(inShadowCoord / inShadowCoord.w);

	vec4 color = texture(texSampler, inUV) * vec4(inColor, 1.0);
	
	vec3 lightColor = vec3(1.0);

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 ambient = 0.1 * lightColor;
	vec3 diffuse = max(dot(N, L), 0.0) * lightColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 64.0) * lightColor;

	vec3 lighting = (ambient + shadow * (diffuse + specular)) * color.rgb;

	outFragColor = vec4(lighting, color.a);
}
