#pragma once

#include "VulkanFramework.h"

class GameObject
{
protected:
	// up은 항상 (0.f, 1.f, 0.f)이라고 가정한다.
	glm::mat4 modelTransform{ 1.f };

protected:
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	VkDescriptorSet samplerDescriptorSet = VK_NULL_HANDLE;

public:
	GameObject();
	virtual ~GameObject();

	virtual void initialize();
	virtual void update(float elapsedTime);
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void release();

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;					// y는 항상 0으로 가정
	virtual glm::vec3 getPosition() const final;
	virtual glm::vec3 getLook() const final;
	virtual glm::vec3 getRight() const final;

	virtual void moveForward(float value) final;
	virtual void move(glm::vec3 direction, float value) final;

	virtual void rotate(float degree) final;

	void setBuffer(vkf::Buffer& buffer);
	void setTexture(vkf::Texture& texture);

};

struct PushConstantData {
	alignas(16) glm::mat4 model;
};
