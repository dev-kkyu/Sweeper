#include <iostream>

#include "GLTFCollisionModel.h"

int main()
{
	GLTFCollisionModel model{ "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb" };

	std::cout << model.vertexBuffer.size() << "���� ���ؽ�" << std::endl;
	std::cout << model.indexBuffer.size() << "���� �ε���" << std::endl;
	std::cout << model.nodes.size() << "���� �θ� ���" << std::endl;
}