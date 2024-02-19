#include "GameObject.h"

#include <stdexcept>

GameObject::GameObject(vkf::Device& fDevice)
	: fDevice{ fDevice }
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
	vkDestroyDescriptorPool(fDevice.device, samplerDescriptorPool, nullptr);

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

void GameObject::setTexture(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture)
{
	createSamplerDescriptorPool();
	createSamplerDescriptorSets(samplerDescriptorSetLayout, texture);
}

void GameObject::createSamplerDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(fDevice.device, &poolInfo, nullptr, &samplerDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void GameObject::createSamplerDescriptorSets(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = samplerDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &samplerDescriptorSetLayout;

	if (vkAllocateDescriptorSets(fDevice.device, &allocInfo, &samplerDescriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture.textureImageView;
	imageInfo.sampler = texture.textureSampler;

	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = samplerDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(fDevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
