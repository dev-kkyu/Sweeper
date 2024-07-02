#include "BoundingBoxObject.h"

BoundingBoxObject::BoundingBoxObject()
{
}

BoundingBoxObject::~BoundingBoxObject()
{
}

void BoundingBoxObject::initialize()
{
}

void BoundingBoxObject::update(float elapsedTime, uint32_t currentFrame)
{
}

void BoundingBoxObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkf::PushConstantData), &modelTransform);
	vkCmdDraw(commandBuffer, 48, 1, 0, 0);
}

void BoundingBoxObject::release()
{
}
