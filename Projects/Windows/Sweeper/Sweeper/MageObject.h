#pragma once

#include "PlayerObject.h"

class MageATTACKState : public StateMachine
{
public:
	MageATTACKState(PlayerObject& player);
	virtual ~MageATTACKState() = default;

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
	friend MageATTACKState;
	friend MageSKILLState;

private:
	struct MageEffect {
		glm::vec3 pos;
		float accumTime = 0.f;
	};

	vkf::Effect& attackEffect;
	vkf::Effect& skillEffect;
	std::vector<MageEffect> mageAttackEffects;
	std::vector<MageEffect> mageSkillEffects;

public:
	MageObject(GLTFModelObject& mapObject, vkf::Effect& aEffect, vkf::Effect& sEffect);
	virtual ~MageObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

