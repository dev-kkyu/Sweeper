#pragma once

#include "GLTFCollisionModel.h"

class CollisionObject
{
private:
	const GLTFCollisionModel& model;

	std::vector<BoundingBox> boundingBox;

public:
	CollisionObject(const GLTFCollisionModel& model);

	const std::vector<BoundingBox>& getBoundingBox() const;

private:
	void copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node);

};

