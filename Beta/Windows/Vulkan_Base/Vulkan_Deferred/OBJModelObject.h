#pragma once

#include "GameObjectBase.h"

class OBJModelObject : public GameObjectBase
{
protected:

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;

	uint32_t indexCount = 0;

	VkDescriptorSet samplerDescriptorSet = VK_NULL_HANDLE;

public:
	OBJModelObject();
	virtual ~OBJModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void drawPos(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	virtual void release() override;

	void setBuffer(const vkf::MeshBuffer& buffer);
	void setTexture(const vkf::Texture& texture);
};

