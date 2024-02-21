#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 lightColor = vec3(1.f, 1.f, 1.f);
	vec3 lightPos = vec3(30.f, 30.f, 30.f);
	vec3 cameraPos = vec3(30.f, 30.f, -30.f);

	float ambientLight = 0.3f;										//--- �ֺ� ���� ���
	vec3 ambient = ambientLight * lightColor;						//--- �ֺ� ���� ��

	vec3 normalVector = normalize(inNormal);						//--- �븻���� ����ȭ�Ѵ�.
	vec3 lightDir = normalize(lightPos - fragPos);					//--- ǥ��� ������ ��ġ�� ������ ������ �����Ѵ�.
	float diffuseLight = max(dot(normalVector, lightDir), 0.0);		//--- N�� L�� ���� ������ ���� ���� (���� ���� ���� �� ���� �Ѵ�.)
	vec3 diffuse = diffuseLight * lightColor;						//--- ��� �ݻ� ���� = ��� �ݻ簪 * ���� ����

	int shininess = 128;											//--- ���� ���
	vec3 cameraDir = normalize (cameraPos - fragPos);				//--- �������� ����
	vec3 reflectDir = reflect (-lightDir, normalVector);			//--- �ݻ� ���� = reflect �Լ� - �Ի� ������ �ݻ� ���� ���
	float specularLight = max (dot(cameraDir, reflectDir), 0.0);	//--- V�� R�� ���� ������ ���� ���� (���� ����)
	specularLight = pow(specularLight, shininess);					//--- shininess ���� ���־� ���̶���Ʈ�� ������ش�.
	vec3 specular = specularLight * lightColor;						//--- �ſ� �ݻ� ���� = �ſ� �ݻ簪 * ���� ����

	vec4 color = texture(texSampler, fragTexCoord);
	vec3 result = (ambient + diffuse + specular) * color.rgb;		//--- ���� ���� ������ �ȼ� ���� = (�ֺ����� + ����ݻ����� + �ſ�ݻ�����) * ��ü ����
	outColor = vec4(result, 1.0);
}
