#pragma once

#include "GLTFSkinModelObject.h"

enum class BOSS_STATE : char;
class BossObject : public GLTFSkinModelObject
{
private:
	struct BossEffect {
		glm::vec3 pos;
		float accumTime = 100.f;
	} bossEffect;

	vkf::Effect& effect;

	bool isDead;
	float deadAccumTime;

public:
	BossObject(vkf::Effect& effect);
	virtual ~BossObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const final;
	virtual void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const override;

	void setBossState(BOSS_STATE state);

};

