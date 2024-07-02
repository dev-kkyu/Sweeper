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
	glm::mat4 modelTransform;

public:
	BoundingBox();
	~BoundingBox();

	void setBound(float top, float bottom, float front, float back, float left, float right);

	void updatePosition(const glm::vec3& position);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);

	bool isCollide(const BoundingBox& other);

};

