#include "BoundingBox.h"

BoundingBox::BoundingBox()
	: modelTransform{ 1.f }
{
	top = 1.f;
	bottom = 0.f;
	front = 0.5f;
	back = -0.5f;
	left = -0.5f;
	right = 0.5f;
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::setBound(float top, float bottom, float front, float back, float left, float right)
{
	this->top = top;
	this->bottom = bottom;
	this->front = front;
	this->back = back;
	this->left = left;
	this->right = right;
}

void BoundingBox::updatePosition(const glm::vec3& position)
{
	float len_x = right - left;
	float len_y = top - bottom;
	float len_z = front - back;

	float cen_x = (right + left) / 2.f;
	float cen_y = (top + bottom) / 2.f;
	float cen_z = (front + back) / 2.f;

	glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), glm::vec3(len_x, len_y, len_z));
	glm::mat4 posMat = glm::translate(glm::mat4(1.f), glm::vec3(cen_x, cen_y, cen_z) + position);

	modelTransform = posMat * scaleMat;
}

void BoundingBox::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkf::PushConstantData), &modelTransform);
	vkCmdDraw(commandBuffer, 48, 1, 0, 0);
}

bool BoundingBox::isCollide(const BoundingBox& other)
{
	if (left > other.right) return false;
	if (right < other.left) return false;
	if (top < other.bottom) return false;
	if (bottom > other.top) return false;
	if (front < other.back) return false;
	if (back > other.front) return false;
	return true;
}
