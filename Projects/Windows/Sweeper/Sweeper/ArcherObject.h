#pragma once

#include "PlayerObject.h"

class ArcherATTACKState : public StateMachine
{
public:
	ArcherATTACKState(PlayerObject& player);
	virtual ~ArcherATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArcherSKILLState : public StateMachine
{
public:
	ArcherSKILLState(PlayerObject& player);
	virtual ~ArcherSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArcherObject : public PlayerObject
{
	friend class ArcherSKILLState;

private:
	struct ArcherEffect {
		glm::vec3 pos;
		glm::vec3 dir;
		float accumTime = 0.f;
	};

	vkf::Effect& effect;
	std::vector<ArcherEffect> archerEffects;

public:
	ArcherObject(GLTFModelObject& mapObject, vkf::Effect& effect);
	virtual ~ArcherObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const override;
	virtual void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

