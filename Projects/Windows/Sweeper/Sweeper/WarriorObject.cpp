#include "WarriorObject.h"

#define PLAYER_CLIP_ATTACK_WARRIOR	16
#define PLAYER_CLIP_SKILL_WARRIOR	12

WarriorAttackState::WarriorAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void WarriorAttackState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_WARRIOR);
	player.setAnimateSpeed(0.4f);
}

void WarriorAttackState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void WarriorAttackState::exit()
{
	StateMachine::exit();
}

WarriorSKILLState::WarriorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void WarriorSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_WARRIOR);
	player.setAnimateSpeed(1.175f);
}

void WarriorSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void WarriorSKILLState::exit()
{
	StateMachine::exit();
}

WarriorObject::WarriorObject(GLTFModelObject& mapObject, vkf::Effect& effect)
	: PlayerObject{ mapObject }, effect{ effect }
{
	effectTimes.resize(1);
}

void WarriorObject::initialize()
{
}

void WarriorObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& time : effectTimes) {
		time += elapsedTime;
	}
}

void WarriorObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void WarriorObject::release()
{
}

void WarriorObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

	for (const float time : effectTimes) {
		glm::mat4 matrix = glm::translate(glm::mat4(1.f), getPosition() + getLook() * 1.5f);
		matrix[3][3] = time;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
}

void WarriorObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<WarriorAttackState>(*this);
	currentState->enter();
}

void WarriorObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<WarriorSKILLState>(*this);
	currentState->enter();
}
