#pragma once

#include "PlayerObject.h"

class ArchorAttackState : public StateMachine
{
public:
	ArchorAttackState(PlayerObject& player);
	virtual ~ArchorAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArchorSKILLState : public StateMachine
{
public:
	ArchorSKILLState(PlayerObject& player);
	virtual ~ArchorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArchorObject : public PlayerObject
{
private:

public:
	ArchorObject(GLTFModelObject& mapObject);
	virtual ~ArchorObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

