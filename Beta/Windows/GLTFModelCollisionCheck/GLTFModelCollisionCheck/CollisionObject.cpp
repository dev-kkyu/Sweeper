#include "CollisionObject.h"

CollisionObject::CollisionObject(const GLTFCollisionModel& model)
	: model{ model }
{
	// �ٿ�� �ڽ��� ������ �´�
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
		if (box.getTop() > 0.f)		// �Ʒ��� �ڽ��� ������� �ʴ´�.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
