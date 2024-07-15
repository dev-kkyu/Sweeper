#pragma once

#include "GLTFModelObject.h"

#include <chrono>

class ArrowObject : public GLTFModelObject
{
private:
	std::chrono::steady_clock::time_point spawnTime;

public:
	ArrowObject();
	virtual ~ArrowObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

};

