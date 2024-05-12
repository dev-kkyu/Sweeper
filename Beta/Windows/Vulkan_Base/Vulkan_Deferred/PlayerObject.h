#pragma once

#define KEY_UP		0x01
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x08

#include "OBJModelObject.h"

class PlayerObject : public OBJModelObject
{
private:

	unsigned int keyState = 0;

	float startXpos = 0.f;
	float startYpos = 0.f;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void processKeyInput(unsigned int keyState);
	void setStartMousePos(float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

};

