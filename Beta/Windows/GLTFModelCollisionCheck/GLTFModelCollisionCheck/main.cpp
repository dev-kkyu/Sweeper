#include <iostream>

#include "CollisionObject.h"

int main()
{
	GLTFCollisionModel model{ "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb" };
	CollisionObject object{ model };

	std::cout << object.getBoundingBox().size() << "���� �ٿ�� �ڽ�" << std::endl;

}