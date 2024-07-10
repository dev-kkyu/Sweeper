#include "MageObject.h"

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
}

void MageSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void MageSKILLState::exit()
{
	StateMachine::exit();
}

MageObject::MageObject(GLTFModelObject& mapObject)
	: PlayerObject{ mapObject }
{
}

void MageObject::initialize()
{
}

void MageObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);
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
