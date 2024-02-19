#pragma once

#include "VulkanFramework.h"

class GameObject
{
protected:
	glm::vec3 position{ 0.f };
	glm::vec3 look{ 0.f, 0.f, 1.f };	// y는 0으로 가정

	glm::mat4 modelTransform{ 1.f };

protected:
	vkf::Device& fDevice;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;

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

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;
	virtual glm::vec3 getPosition() const final;
	virtual glm::vec3 getLook() const final;

	virtual void moveForward(float value) final;
	virtual void move(glm::vec3 direction, float value) final;

	void setBuffer(vkf::Buffer& buffer);
	void setTexture(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture);

protected:
	void createSamplerDescriptorPool();
	void createSamplerDescriptorSets(VkDescriptorSetLayout samplerDescriptorSetLayout, vkf::Texture texture);
};

struct PushConstantData {
	alignas(16) glm::mat4 model;
};
