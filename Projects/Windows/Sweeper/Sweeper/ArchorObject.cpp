#include "ArchorObject.h"

#define PLAYER_CLIP_ATTACK_ARCHOR	26
#define PLAYER_CLIP_SKILL_ARCHOR	26

ArchorAttackState::ArchorAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArchorAttackState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_ARCHOR);
	player.setAnimateSpeed(0.8f);
}

void ArchorAttackState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArchorAttackState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void ArchorAttackState::exit()
{
	StateMachine::exit();
}

ArchorSKILLState::ArchorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArchorSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_ARCHOR);
	player.setAnimateSpeed(0.8f);
}

void ArchorSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArchorSKILLState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void ArchorSKILLState::exit()
{
	StateMachine::exit();
}

ArchorObject::ArchorObject(GLTFModelObject& mapObject)
	: PlayerObject{ mapObject }
{
}

void ArchorObject::initialize()
{
}

void ArchorObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);
}

void ArchorObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void ArchorObject::release()
{
}

void ArchorObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void ArchorObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<ArchorAttackState>(*this);
	currentState->enter();
}

void ArchorObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<ArchorSKILLState>(*this);
	currentState->enter();
}
