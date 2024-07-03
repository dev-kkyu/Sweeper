#pragma once

#include "VulkanFramework.h"

class BoundingBox
{
private:
	float top;
	float bottom;
	float front;
	float back;
	float left;
	float right;

private:
	glm::mat4 shaderTransform;

public:
	BoundingBox();
	~BoundingBox();

	void setBound(float top, float bottom, float front, float back, float left, float right);
	void applyTransform(const glm::mat4& transform);

	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	bool isCollide(const BoundingBox& other);

	float getTop() const;
	float getBottom() const;
	float getFront() const;
	float getBack() const;
	float getLeft() const;
	float getRight() const;

};

