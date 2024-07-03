#include "Map.h"

#define MAP_NAME "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb"

Map::Map()
	: model{ MAP_NAME }
{
	// 바운딩 박스를 복사해 온다
	for (const auto& node : this->model.nodes) {
		copyBoundingBoxByNode(node);
	}
}

Map::~Map()
{
}

Map& Map::getInstance()
{
	static Map instance;		// C++11 이후 Thread-Safe 하다.
	return instance;
}

const std::vector<BoundingBox>& Map::getBoundingBox() const
{
	return boundingBox;
}

void Map::copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node)
{
	for (const auto& box : node->mesh.boundingBox) {
		if (box.getTop() > 0.f)		// 아래쪽 박스는 사용하지 않는다.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
