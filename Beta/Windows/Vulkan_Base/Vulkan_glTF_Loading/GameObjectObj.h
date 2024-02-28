#pragma once

#include "GameObjectBase.h"

class GameObjectObj : public GameObjectBase
{
protected:

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;

	uint32_t indexCount = 0;

	VkDescriptorSet samplerDescriptorSet = VK_NULL_HANDLE;

public:
	GameObjectObj();
	virtual ~GameObjectObj();

	virtual void initialize();
	virtual void update(float elapsedTime);
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void release();

	void setBuffer(vkf::Buffer& buffer);
	void setTexture(vkf::Texture& texture);
};

