#include "GameObjectObj.h"

GameObjectObj::GameObjectObj()
{
}

GameObjectObj::~GameObjectObj()
{
}

void GameObjectObj::initialize()
{
}

void GameObjectObj::update(float elapsedTime)
{
}

void GameObjectObj::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
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

void GameObjectObj::release()
{
}

void GameObjectObj::setBuffer(vkf::Buffer& buffer)
{
	vertexBuffer = buffer.vertexBuffer;
	indexBuffer = buffer.indexBuffer;

	vertexCount = static_cast<uint32_t>(buffer.vertices.size());
	indexCount = static_cast<uint32_t>(buffer.indices.size());
}

void GameObjectObj::setTexture(vkf::Texture& texture)
{
	samplerDescriptorSet = texture.samplerDescriptorSet;
}
