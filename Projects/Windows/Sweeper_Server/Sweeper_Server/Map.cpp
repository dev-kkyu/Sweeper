#include "Map.h"

#define MAP_NAME "../../../../Projects/Windows/Sweeper/Sweeper/Models/map.glb"

Map::Map()
	: model{ MAP_NAME }
{
	// �ٿ�� �ڽ��� ������ �´�
	for (const auto& node : this->model.nodes) {
		copyBoundingBoxByNode(node);
	}
}

Map::~Map()
{
}

Map& Map::getInstance()
{
	static Map instance;		// C++11 ���� Thread-Safe �ϴ�.
	return instance;
}

const std::vector<BoundingBox>& Map::getBoundingBox() const
{
	return boundingBox;
}

void Map::copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node)
{
	for (const auto& box : node->mesh.boundingBox) {
		if (box.getTop() > 0.f)		// �Ʒ��� �ڽ��� ������� �ʴ´�.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
