#include <iostream>

#include "CollisionObject.h"

int main()
{
	GLTFCollisionModel model{ "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb" };
	CollisionObject object{ model };

	std::cout << object.getBoundingBox().size() << "개의 바운딩 박스" << std::endl;

}