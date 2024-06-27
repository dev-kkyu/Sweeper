#pragma once

#include "GLTFSkinModelObject.h"

#define PLAYER_CLIP_IDLE			19
#define PLAYER_CLIP_RUN				24
#define PLAYER_CLIP_DASH			7
#define PLAYER_CLIP_ATTACK_KNIFE	10

class PlayerObject : public GLTFSkinModelObject
{
private:
	// 키가 눌려진 상태를 종합한다.
	unsigned int keyState = 0;

	// 움직이는 속도	// 값은 생성자에서
	float maxMoveSpeed;		// 최대 속도
	float moveSpeed;		// 현재 속도
	float acceleration;		// 현재 가속도
	glm::vec3 lastDirection;	// 움직이던 방향 (키를 뗐을 때 갈 방향, xz만 사용)

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void processKeyInput(unsigned int key, bool is_pressed);

};
