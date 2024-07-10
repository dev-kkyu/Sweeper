#include "WarriorObject.h"

#define PLAYER_CLIP_ATTACK_KNIFE	16

WarriorAttackState::WarriorAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void WarriorAttackState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_KNIFE);
	player.setAnimateSpeed(0.4f);
}

void WarriorAttackState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void WarriorAttackState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void WarriorAttackState::exit()
{
	StateMachine::exit();
}

WarriorSKILLState::WarriorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
}

void WarriorSKILLState::enter()
{
	StateMachine::enter();
}

void WarriorSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void WarriorSKILLState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void WarriorSKILLState::exit()
{
	StateMachine::exit();
}

WarriorObject::WarriorObject(GLTFModelObject& mapObject)
	: PlayerObject{ mapObject }
{
}

void WarriorObject::initialize()
{
}

void WarriorObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);
}

void WarriorObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void WarriorObject::release()
{
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
