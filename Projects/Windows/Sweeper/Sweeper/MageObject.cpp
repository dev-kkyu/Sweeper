#include "MageObject.h"

#define PLAYER_CLIP_ATTACK_MAGE		10
#define PLAYER_CLIP_SKILL_MAGE		11

#include "SoundManager.h"

MageATTACKState::MageATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void MageATTACKState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_MAGE);
	player.setAnimateSpeed(1.f);

	dynamic_cast<MageObject*>(&player)->mageAttackEffects.push_back(MageObject::MageEffect{ player.getPosition() + player.getLook() * 3.f, -0.23f });

	SoundManager::getInstance().playMageAttackSound();
}

void MageATTACKState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void MageATTACKState::exit()
{
	StateMachine::exit();
}

MageSKILLState::MageSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void MageSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_MAGE);
	player.setAnimateSpeed(1.f);

	dynamic_cast<MageObject*>(&player)->mageSkillEffects.push_back(MageObject::MageEffect{ player.getPosition() + player.getLook() * 4.f, -0.3f });

	SoundManager::getInstance().playMageSkillSound();
}

void MageSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void MageSKILLState::exit()
{
	StateMachine::exit();
}

MageObject::MageObject(GLTFModelObject& mapObject, vkf::Effect& aEffect, vkf::Effect& sEffect)
	: PlayerObject{ mapObject }, attackEffect{ aEffect }, skillEffect{ sEffect }
{
	player_type = PLAYER_TYPE::MAGE;

	maxHP = HP = MAX_HP_PLAYER_MAGE;
}

void MageObject::initialize()
{
}

void MageObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& mEffect : mageAttackEffects) {
		mEffect.accumTime += elapsedTime;
	}

	for (auto& mEffect : mageSkillEffects) {
		mEffect.accumTime += elapsedTime;
	}

	std::list<std::list<MageObject::MageEffect>::iterator> deleteEffects;
	for (auto itr = mageAttackEffects.begin(); itr != mageAttackEffects.end(); ++itr) {
		if (itr->accumTime >= 0.625f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		mageAttackEffects.erase(itr);
	}
	deleteEffects.clear();
	for (auto itr = mageSkillEffects.begin(); itr != mageSkillEffects.end(); ++itr) {
		if (itr->accumTime >= 1.f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		mageSkillEffects.erase(itr);
	}
}

void MageObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void MageObject::release()
{
}

void MageObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	if (mageAttackEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, attackEffect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &attackEffect.texture.samplerDescriptorSet, 0, nullptr);

		for (const auto& mEffect : mageAttackEffects) {
			if (mEffect.accumTime >= 0.f) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.f), mEffect.pos);
				matrix[3][3] = mEffect.accumTime;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			}
		}
	}

	if (mageSkillEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skillEffect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &skillEffect.texture.samplerDescriptorSet, 0, nullptr);

		for (const auto& mEffect : mageSkillEffects) {
			if (mEffect.accumTime >= 0.f) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.f), mEffect.pos);
				matrix[3][3] = mEffect.accumTime;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			}
		}
	}
}

void MageObject::drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	GLTFSkinModelObject::drawUI(commandBuffer, pipelineLayout);
}

void MageObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<MageATTACKState>(*this);
	currentState->enter();
}

void MageObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<MageSKILLState>(*this);
	currentState->enter();
}
