#include "RotateObject.h"

RotateObject::RotateObject(vkf::Device& fDevice) : GameObject{ fDevice }
{
}

RotateObject::~RotateObject()
{
}

void RotateObject::initialize()
{
}

void RotateObject::update(float elapsedTime)
{
	// 초당 90도 회전
	rotateAngle += elapsedTime * rotateSpeed;

	modelTransform = glm::rotate(glm::mat4(1.f), glm::radians(rotateAngle), glm::vec3(0.f, 1.f, 0.f));
	modelTransform = glm::translate(glm::mat4(1.f), position) * modelTransform;
}

void RotateObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantData), &modelTransform);

	// set = 1에 샘플러 바인드
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &samplerDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void RotateObject::release()
{
}

void RotateObject::setRotateSpeed(float speed)
{
	rotateSpeed = speed;
}
