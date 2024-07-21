#pragma once

#include "GLTFSkinModelObject.h"

enum class BOSS_STATE : char;
class BossObject : public GLTFSkinModelObject
{
private:

public:
	BossObject();
	virtual ~BossObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setBossState(BOSS_STATE state);

};

