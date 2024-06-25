#pragma once

#include "GLTFSkinModelObject.h"

#include "NetworkManager.h"			// Player State

class PlayerObject : public GLTFSkinModelObject
{
private:
	int my_id;

	PLAYER_STATE state;

	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

	// �����̴� �ӵ�
	float moveSpeed = 0.f;

public:
	PlayerObject();
	PlayerObject(int player_id);
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setPlayerID(int player_id);

	void setPlayerState(PLAYER_STATE state);
	PLAYER_STATE getPlayerState() const;

};
