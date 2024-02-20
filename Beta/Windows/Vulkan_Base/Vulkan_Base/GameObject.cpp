#include "GameObject.h"

#include <stdexcept>

#include <glm/gtx/vector_angle.hpp>

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
	modelTransform[3] = glm::vec4(position, 1.f);
}

void GameObject::setLook(glm::vec3 look)
{
	glm::vec3 position = getPosition();

	glm::vec3 baseDir{ 0.f, 0.f, 1.f };
	glm::vec3 normalizedLook = glm::normalize(look);
	float radianAngle = glm::orientedAngle(baseDir, normalizedLook, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.f), radianAngle, glm::vec3(0.f, 1.f, 0.f));

	modelTransform = rotateMat;
	setPosition(position);
}

glm::vec3 GameObject::getPosition() const
{
	return glm::vec3(modelTransform[3]);
}

glm::vec3 GameObject::getLook() const
{
	return glm::normalize(glm::vec3(modelTransform[2]));
}

void GameObject::moveForward(float value)
{
	auto position = getPosition();
	position += getLook() * value;
	setPosition(position);
}

void GameObject::move(glm::vec3 direction, float value)
{
	auto position = getPosition();
	position += glm::normalize(direction) * value;
	setPosition(position);
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

