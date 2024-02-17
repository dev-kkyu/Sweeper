#pragma once

#include "VulkanFramework.h"

class GameObject
{
protected:
	glm::vec3 position;
	glm::vec3 lookForward;

protected:
	vkf::Device& fDevice;

protected:
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	VkDescriptorPool samplerDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet samplerDescriptorSet;

public:
	GameObject(vkf::Device& fDevice);
	virtual ~GameObject();

	virtual void initialize();
	virtual void update(float elapsedTime);
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void release();

	void setBuffer(std::vector<vkf::Vertex> vertices, std::vector<uint32_t> indices);
	void setTexture(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture);

protected:
	void createVertexBuffer(std::vector<vkf::Vertex> vertices);
	void createIndexBuffer(std::vector<uint32_t> indices);
	void createSamplerDescriptorPool();
	void createSamplerDescriptorSets(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture);
};

struct PushConstantData {
	alignas(16) glm::mat4 model;
};
