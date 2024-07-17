#pragma once

#include "PlayerObject.h"

class WarriorATTACKState : public StateMachine
{
public:
	WarriorATTACKState(PlayerObject& player);
	virtual ~WarriorATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class WarriorSKILLState : public StateMachine
{
public:
	WarriorSKILLState(PlayerObject& player);
	virtual ~WarriorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class WarriorObject : public PlayerObject
{
	friend class WarriorSKILLState;

private:
	struct WarriorEffect {
		glm::vec3 pos;
		float accumTime = 0.f;
	};

	vkf::Effect& effect;
	std::vector<WarriorEffect> warriorEffects;

public:
	WarriorObject(GLTFModelObject& mapObject, vkf::Effect& effect);
	virtual ~WarriorObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

