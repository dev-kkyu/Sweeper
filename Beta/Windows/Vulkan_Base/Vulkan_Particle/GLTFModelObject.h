#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFModel.h"

class GLTFModelObject : public GameObjectBase
{
protected:

	VulkanGLTFModel* model = nullptr;

public:
	GLTFModelObject();
	virtual ~GLTFModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setModel(VulkanGLTFModel& model);

};

