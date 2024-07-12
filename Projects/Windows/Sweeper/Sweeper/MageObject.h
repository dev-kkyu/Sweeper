#pragma once

#include "PlayerObject.h"

class MageAttackState : public StateMachine
{
public:
	MageAttackState(PlayerObject& player);
	virtual ~MageAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class MageSKILLState : public StateMachine
{
public:
	MageSKILLState(PlayerObject& player);
	virtual ~MageSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class MageObject : public PlayerObject
{
	friend MageSKILLState;

private:
	struct MageEffect {
		glm::vec3 pos;
		float accumTime = 0.f;
	};

	vkf::Effect& effect;
	std::vector<MageEffect> mageEffects;

public:
	MageObject(GLTFModelObject& mapObject, vkf::Effect& effect);
	virtual ~MageObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

