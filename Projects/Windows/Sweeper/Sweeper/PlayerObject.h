#pragma once

#define KEY_UP		0x01
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x08

#include "GLTFSkinModelObject.h"

class PlayerObject : public GLTFSkinModelObject
{
private:
	// 플레이어가 회전하기 위한 마우스 클릭 발생 시 시작 위치
	float startXpos = 0.f;
	float startYpos = 0.f;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setStartMousePos(float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

};

