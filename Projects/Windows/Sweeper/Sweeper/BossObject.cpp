#include "BossObject.h"

#include "NetworkManager.h"			// BOSS_STATE

#include <iostream>

#define BOSS_CLIP_SLEEP			0
#define BOSS_CLIP_WAKE_UP		6
#define BOSS_CLIP_IDLE			5
#define BOSS_CLIP_MOVE			8
#define BOSS_CLIP_LEFT_PUNCH	1
#define BOSS_CLIP_RIGHT_PUNCH	7
#define BOSS_CLIP_PUNCH_DOWN	2
#define BOSS_CLIP_DIE			3

BossObject::BossObject()
{
	activeAnimation = BOSS_CLIP_SLEEP;

	maxHP = HP = MAX_HP_BOSS;
}

void BossObject::initialize()
{
}

void BossObject::update(float elapsedTime, uint32_t currentFrame)
{
	GLTFSkinModelObject::update(elapsedTime, currentFrame);
}

void BossObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void BossObject::release()
{
}

void BossObject::setBossState(BOSS_STATE state)
{
	switch (state) {
	case BOSS_STATE::SLEEP:
		setAnimationClip(BOSS_CLIP_SLEEP);
		setAnimateSpeed(1.f);
		break;
	case BOSS_STATE::WAKEUP:
		setAnimationClip(BOSS_CLIP_WAKE_UP);
		setAnimateSpeed(0.7f);
		break;
	case BOSS_STATE::IDLE:
		setAnimationClip(BOSS_CLIP_IDLE);
		setAnimateSpeed(0.7f);
		break;
	case BOSS_STATE::MOVE:
		setAnimationClip(BOSS_CLIP_MOVE);
		setAnimateSpeed(0.7f);
		break;
	case BOSS_STATE::LEFT_PUNCH:
		setAnimationClip(BOSS_CLIP_LEFT_PUNCH);
		setAnimateSpeed(0.7f);
		break;
	case BOSS_STATE::RIGHT_PUNCH:
		setAnimationClip(BOSS_CLIP_RIGHT_PUNCH);
		setAnimateSpeed(0.6f);
		break;
	case BOSS_STATE::PUNCH_DOWN:
		setAnimationClip(BOSS_CLIP_PUNCH_DOWN);
		setAnimateSpeed(0.55f);
		break;
	case BOSS_STATE::DIE:
		setAnimationClip(BOSS_CLIP_DIE);
		setAnimateSpeed(0.7f);
		break;
	default:
		std::cerr << "ERROR : INVALID BOSS STATE!" << std::endl;
		break;
	}
}
