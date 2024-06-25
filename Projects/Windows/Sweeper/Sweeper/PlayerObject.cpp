#include "PlayerObject.h"

#include <iostream>

#define CLIP_IDLE			19
#define CLIP_RUN			24
#define CLIP_ATTACK_KNIFE	10

PlayerObject::PlayerObject()
{
	my_id = -1;

	state = PLAYER_STATE::IDLE;

	moveSpeed = PLAYER_SPEED;							// �ʴ� �̵��ӵ� 5m
}

PlayerObject::PlayerObject(int player_id)
	: PlayerObject{}
{
	my_id = player_id;
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime, uint32_t currentFrame)
{
	// Ű �Է¿� ���� �������� �������� ó��

	GLTFSkinModelObject::update(elapsedTime, currentFrame);
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void PlayerObject::release()
{
}

void PlayerObject::setPlayerID(int player_id)
{
	my_id = player_id;
}

void PlayerObject::setPlayerState(PLAYER_STATE state)
{
	this->state = state;

	if (state == PLAYER_STATE::IDLE)
		setAnimationClip(CLIP_IDLE);
	else if (state == PLAYER_STATE::RUN)
		setAnimationClip(CLIP_RUN);
	else if (state == PLAYER_STATE::ATTACK)
		setAnimationClip(CLIP_ATTACK_KNIFE);
	else
		std::cout << int(my_id) << ": STATE ����" << std::endl;
	std::cout << int(my_id) << "�� ���°� " << ((state == PLAYER_STATE::RUN) ? "RUN" : (state == PLAYER_STATE::IDLE) ? "IDLE" : "ATTACK") << "�� ����" << std::endl;
}

PLAYER_STATE PlayerObject::getPlayerState() const
{
	return state;
}
