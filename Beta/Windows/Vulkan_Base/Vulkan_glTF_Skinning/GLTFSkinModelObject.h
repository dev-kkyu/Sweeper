#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFSkinModel.h"

class GLTFSkinModelObject : public GameObjectBase
{
protected:

	VulkanGLTFSkinModel* model = nullptr;

public:
	GLTFSkinModelObject();
	virtual ~GLTFSkinModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setModel(VulkanGLTFSkinModel& model);

};

