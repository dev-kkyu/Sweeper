#pragma once

#include "PlayerObject.h"

class WarriorAttackState : public StateMachine
{
public:
	WarriorAttackState(PlayerObject& player);
	virtual ~WarriorAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class WarriorSKILLState : public StateMachine
{
public:
	WarriorSKILLState(PlayerObject& player);
	virtual ~WarriorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class WarriorObject : public PlayerObject
{
private:

public:
	WarriorObject(GLTFModelObject& mapObject);
	virtual ~WarriorObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

