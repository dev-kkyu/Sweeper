#include "MageObject.h"

#include <algorithm>

#define PLAYER_CLIP_ATTACK_MAGE		10
#define PLAYER_CLIP_SKILL_MAGE		11

MageAttackState::MageAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void MageAttackState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_MAGE);
	player.setAnimateSpeed(1.f);
}

void MageAttackState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void MageAttackState::exit()
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

	dynamic_cast<MageObject*>(&player)->mageEffects.push_back(MageObject::MageEffect{ player.getPosition() + player.getLook() * 3.f, -0.3f });
}

void MageSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void MageSKILLState::exit()
{
	StateMachine::exit();
}

MageObject::MageObject(GLTFModelObject& mapObject, vkf::Effect& effect)
	: PlayerObject{ mapObject }, effect{ effect }
{
}

void MageObject::initialize()
{
}

void MageObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& mEffect : mageEffects) {
		mEffect.accumTime += elapsedTime;
	}

	std::list<std::vector<MageObject::MageEffect>::iterator> deleteEffects;
	for (auto itr = mageEffects.begin(); itr != mageEffects.end(); ++itr) {
		if (itr->accumTime >= 1.f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		mageEffects.erase(itr);
	}
}

void MageObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void MageObject::release()
{
}

void MageObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (mageEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

		for (const auto& mEffect : mageEffects) {
			if (mEffect.accumTime >= 0.f) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.f), mEffect.pos);
				matrix[3][3] = mEffect.accumTime;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			}
		}
	}
}

void MageObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<MageAttackState>(*this);
	currentState->enter();
}

void MageObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<MageSKILLState>(*this);
	currentState->enter();
}
