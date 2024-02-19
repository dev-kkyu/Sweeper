#pragma once

#include "GameObject.h"

class RotateObject : public GameObject
{
private:
	// 초당 회전 각도
	float rotateSpeed = 90.f;
	float rotateAngle = 0.f;

public:
	RotateObject();
	virtual ~RotateObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) override;
	virtual void release() override;

	void setRotateSpeed(float speed);
};

