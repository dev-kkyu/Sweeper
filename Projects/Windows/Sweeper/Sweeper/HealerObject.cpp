#include "HealerObject.h"

#define PLAYER_CLIP_ATTACK_HEALER	16
#define PLAYER_CLIP_SKILL_HEALER	31

HealerAttackState::HealerAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void HealerAttackState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_HEALER);
	player.setAnimateSpeed(0.4f);
}

void HealerAttackState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void HealerAttackState::exit()
{
	StateMachine::exit();
}

HealerSKILLState::HealerSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void HealerSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_HEALER);
	player.setAnimateSpeed(1.f);
}

void HealerSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void HealerSKILLState::exit()
{
	StateMachine::exit();
}

HealerObject::HealerObject(GLTFModelObject& mapObject)
	: PlayerObject{ mapObject }
{
}

void HealerObject::initialize()
{
}

void HealerObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);
}

void HealerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void HealerObject::release()
{
}

void HealerObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void HealerObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<HealerAttackState>(*this);
	currentState->enter();
}

void HealerObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<HealerSKILLState>(*this);
	currentState->enter();
}
