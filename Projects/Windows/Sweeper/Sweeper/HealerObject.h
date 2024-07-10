#pragma once

#include "PlayerObject.h"

class HealerAttackState : public StateMachine
{
public:
	HealerAttackState(PlayerObject& player);
	virtual ~HealerAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class HealerSKILLState : public StateMachine
{
public:
	HealerSKILLState(PlayerObject& player);
	virtual ~HealerSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void exit() override;
};

class HealerObject : public PlayerObject
{
private:

public:
	HealerObject(GLTFModelObject& mapObject);
	virtual ~HealerObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

