#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFModel.h"

class GameObjectglTF : public GameObjectBase
{
protected:

	VulkanGLTFModel* model = nullptr;

public:
	GameObjectglTF();
	virtual ~GameObjectglTF();

	virtual void initialize();
	virtual void update(float elapsedTime);
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void release();

	void setModel(VulkanGLTFModel& model);

};

