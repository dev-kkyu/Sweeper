#pragma once

#include "GLTFSkinModelObject.h"

class PlayerObject : public GLTFSkinModelObject
{
private:
	// 키가 눌려진 상태를 종합한다.
	unsigned int keyState = 0;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void processKeyInput(unsigned int key, bool is_pressed);

};
