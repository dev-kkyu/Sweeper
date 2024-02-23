#pragma once

#include "GameObjectBase.h"
#include "VulkanglTFModel.h"

class GameObjectglTF : public GameObjectBase
{
protected:

	VulkanglTFModel* model = nullptr;

public:
	GameObjectglTF();
	virtual ~GameObjectglTF();

	virtual void initialize();
	virtual void update(float elapsedTime);
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void release();

	void setModel(VulkanglTFModel& model);

};

