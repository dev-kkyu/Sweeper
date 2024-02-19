#include "GameObject.h"

#include <stdexcept>

GameObject::GameObject()
{
	initialize();
}

GameObject::~GameObject()
{
	release();
}

void GameObject::initialize()
{
}

void GameObject::update(float elapsedTime)
{
	modelTransform = glm::translate(glm::mat4(1.f), position);
}

void GameObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
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

void GameObject::release()
{

}

void GameObject::setPosition(glm::vec3 position)
{
	this->position = position;
}

void GameObject::setLook(glm::vec3 look)
{
	this->look = look;
}

glm::vec3 GameObject::getPosition() const
{
	return position;
}

glm::vec3 GameObject::getLook() const
{
	return look;
}

void GameObject::moveForward(float value)
{
	position += glm::normalize(look) * value;
}

void GameObject::move(glm::vec3 direction, float value)
{
	position += glm::normalize(direction) * value;
}

void GameObject::setBuffer(vkf::Buffer& buffer)
{
	vertexBuffer = buffer.vertexBuffer;
	indexBuffer = buffer.indexBuffer;

	vertexCount = static_cast<uint32_t>(buffer.vertices.size());
	indexCount = static_cast<uint32_t>(buffer.indices.size());
}

void GameObject::setTexture(vkf::Texture& texture)
{
	samplerDescriptorSet = texture.samplerDescriptorSet;
}

