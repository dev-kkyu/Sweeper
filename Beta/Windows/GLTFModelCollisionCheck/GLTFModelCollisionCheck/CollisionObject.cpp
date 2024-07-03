#include "CollisionObject.h"

CollisionObject::CollisionObject(const GLTFCollisionModel& model)
	: model{ model }
{
	// 바운딩 박스를 복사해 온다
	for (const auto& node : this->model.nodes) {
		copyBoundingBoxByNode(node);
	}
}

const std::vector<BoundingBox>& CollisionObject::getBoundingBox() const
{
	return boundingBox;
}

void CollisionObject::copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node)
{
	for (const auto& box : node->mesh.boundingBox) {
		if (box.getTop() > 0.f)		// 아래쪽 박스는 사용하지 않는다.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
