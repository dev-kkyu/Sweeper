#include "OBJModelObject.h"

OBJModelObject::OBJModelObject()
{
}

OBJModelObject::~OBJModelObject()
{
}

void OBJModelObject::initialize()
{
}

void OBJModelObject::update(float elapsedTime, uint32_t currentFrame)
{
}

void OBJModelObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkf::PushConstantData), &modelTransform);

	// set = 1에 샘플러 바인드
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &samplerDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void OBJModelObject::drawPos(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void OBJModelObject::release()
{
}

void OBJModelObject::setBuffer(const vkf::MeshBuffer& buffer)
{
	vertexBuffer = buffer.vertexBuffer;
	indexBuffer = buffer.indexBuffer;

	indexCount = buffer.indexCount;
}

void OBJModelObject::setTexture(const vkf::Texture& texture)
{
	samplerDescriptorSet = texture.samplerDescriptorSet;
}
