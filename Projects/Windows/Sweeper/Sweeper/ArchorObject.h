#pragma once

#include "PlayerObject.h"

class ArchorATTACKState : public StateMachine
{
public:
	ArchorATTACKState(PlayerObject& player);
	virtual ~ArchorATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArchorSKILLState : public StateMachine
{
public:
	ArchorSKILLState(PlayerObject& player);
	virtual ~ArchorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class ArchorObject : public PlayerObject
{
	friend class ArchorSKILLState;

private:
	struct ArchorEffect {
		glm::vec3 pos;
		glm::vec3 dir;
		float accumTime = 0.f;
	};

	vkf::Effect& effect;
	std::vector<ArchorEffect> archorEffects;

public:
	ArchorObject(GLTFModelObject& mapObject, vkf::Effect& effect);
	virtual ~ArchorObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

