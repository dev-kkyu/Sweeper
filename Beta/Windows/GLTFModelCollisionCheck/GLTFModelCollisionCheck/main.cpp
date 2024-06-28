#include <iostream>

#include "GLTFCollisionModel.h"

int main()
{
	GLTFCollisionModel model{ "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb" };

	std::cout << model.vertexBuffer.size() << "개의 버텍스" << std::endl;
	std::cout << model.indexBuffer.size() << "개의 인덱스" << std::endl;
	std::cout << model.nodes.size() << "개의 부모 노드" << std::endl;
}