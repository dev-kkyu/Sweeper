#pragma once

#define KEY_UP		0x01
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x08

#include "GameObject.h"

class PlayerObject : public GameObject
{
private:

	unsigned int keyState = 0;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) override;
	virtual void release() override;

	void processKeyInput(unsigned int keyState);
};

