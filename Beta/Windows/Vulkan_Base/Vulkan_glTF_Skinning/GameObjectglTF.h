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

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setModel(VulkanglTFModel& model);

};

