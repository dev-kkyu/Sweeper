#pragma once

#include "PlayerObject.h"

class HealerATTACKState : public StateMachine
{
public:
	HealerATTACKState(PlayerObject& player);
	virtual ~HealerATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class HealerSKILLState : public StateMachine
{
public:
	HealerSKILLState(PlayerObject& player);
	virtual ~HealerSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class HealerObject : public PlayerObject
{
	friend class HealerSKILLState;

private:
	struct HealerEffect {
		glm::vec3 pos;
		float accumTime = 0.f;
	};

	vkf::Effect& effect;
	std::vector<HealerEffect> healerEffects;

public:
	HealerObject(GLTFModelObject& mapObject, vkf::Effect& effect);
	virtual ~HealerObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const override;

	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

