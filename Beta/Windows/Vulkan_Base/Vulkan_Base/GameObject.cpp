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

void GameObject::update(float elapsedTime, uint32_t currentFrame)
{

}

void GameObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	// set = 1에 샘플러 바인드
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &samplerDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void GameObject::release()
{
	vkDestroyDescriptorPool(fDevice.device, samplerDescriptorPool, nullptr);

	vkDestroyBuffer(fDevice.device, indexBuffer, nullptr);
	vkFreeMemory(fDevice.device, indexBufferMemory, nullptr);

	vkDestroyBuffer(fDevice.device, vertexBuffer, nullptr);
	vkFreeMemory(fDevice.device, vertexBufferMemory, nullptr);
}

void GameObject::setBuffer(std::vector<vkf::Vertex> vertices, std::vector<uint32_t> indices)
{
	vertexCount = static_cast<uint32_t>(vertices.size());
	indexCount = static_cast<uint32_t>(indices.size());

	createVertexBuffer(vertices);
	createIndexBuffer(indices);
}

void GameObject::setTexture(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture)
{
	createSamplerDescriptorPool();
	createSamplerDescriptorSets(samplerDescriptorSetLayout, texture);
}

void GameObject::createVertexBuffer(std::vector<vkf::Vertex> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(fDevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(fDevice.device, stagingBufferMemory);

	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	vkf::copyBuffer(fDevice, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(fDevice.device, stagingBuffer, nullptr);
	vkFreeMemory(fDevice.device, stagingBufferMemory, nullptr);
}

void GameObject::createIndexBuffer(std::vector<uint32_t> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(fDevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(fDevice.device, stagingBufferMemory);

	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	vkf::copyBuffer(fDevice, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(fDevice.device, stagingBuffer, nullptr);
	vkFreeMemory(fDevice.device, stagingBufferMemory, nullptr);
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
