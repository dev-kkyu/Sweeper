#pragma once

#include "GLTFSkinModelObject.h"

#define PLAYER_CLIP_IDLE			19
#define PLAYER_CLIP_RUN				24
#define PLAYER_CLIP_DASH			7
#define PLAYER_CLIP_ATTACK_KNIFE	10

class PlayerObject : public GLTFSkinModelObject
{
private:
	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

	// �����̴� �ӵ�	// ���� �����ڿ���
	float maxMoveSpeed;		// �ִ� �ӵ�
	float moveSpeed;		// ���� �ӵ�
	float acceleration;		// ���� ���ӵ�
	glm::vec3 lastDirection;	// �����̴� ���� (Ű�� ���� �� �� ����, xz�� ���)

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void processKeyInput(unsigned int key, bool is_pressed);

};
