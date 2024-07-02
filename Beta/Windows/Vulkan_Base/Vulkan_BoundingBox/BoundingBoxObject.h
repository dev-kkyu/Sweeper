#pragma once

#include "GameObjectBase.h"

class BoundingBoxObject : public GameObjectBase
{
private:

public:
	BoundingBoxObject();
	virtual ~BoundingBoxObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

};

