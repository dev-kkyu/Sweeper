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

BossObject::BossObject(vkf::Effect& effect)
	: effect{ effect }
{
	activeAnimation = BOSS_CLIP_SLEEP;

	maxHP = HP = MAX_HP_BOSS;

	isDead = false;
	deadAccumTime = 0.f;
}

void BossObject::initialize()
{
}

void BossObject::update(float elapsedTime, uint32_t currentFrame)
{
	if (isDead)
		deadAccumTime += elapsedTime;
	if (not isDead or deadAccumTime < 5.f)
		GLTFSkinModelObject::update(elapsedTime, currentFrame);

	// 이동 보정
	if (BOSS_CLIP_MOVE == activeAnimation) {
		moveForward(2.f * elapsedTime);		// Todo : 서버와 공유 변수 필요 (이동 속도)
	}

	// 이펙트 누적시간 업데이트
	bossEffect.accumTime += elapsedTime;
}

void BossObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void BossObject::release()
{
}

void BossObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	if (bossEffect.accumTime >= 0.f and bossEffect.accumTime < 1.32f) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

		glm::mat4 matrix = glm::translate(glm::mat4(1.f), bossEffect.pos);
		matrix[3][3] = bossEffect.accumTime;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
}

void BossObject::drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	glm::mat4 matrix{ 1.f };
	if (HP < maxHP / 2.f)
		matrix[3] = glm::vec4(glm::vec3(1.f, 0.f, 0.f), float(HP) / float(maxHP));		// 색상과 체력 게이지
	else
		matrix[3] = glm::vec4(glm::vec3(1.f, 0.1f, 0.f), float(HP) / float(maxHP));		// 색상과 체력 게이지
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
	vkCmdDraw(commandBuffer, 12, 1, 0, 0);
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
		// 보스 스킬 이펙트 발동
		bossEffect = BossEffect{ getPosition(), 0.f };
		break;
	case BOSS_STATE::DIE:
		setAnimationClip(BOSS_CLIP_DIE);
		setAnimateSpeed(0.5f);
		isDead = true;
		deadAccumTime = 0.f;
		break;
	default:
		std::cerr << "ERROR : INVALID BOSS STATE!" << std::endl;
		break;
	}
}
