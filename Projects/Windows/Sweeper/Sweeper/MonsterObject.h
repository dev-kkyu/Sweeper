#pragma once

#include "GLTFSkinModelObject.h"

#define MONSTER_CLIP_IDLE	0
#define MONSTER_CLIP_MOVE	1
#define MONSTER_CLIP_HIT	2
#define MONSTER_CLIP_DIE	3
#define MONSTER_CLIP_ATTACK	4

class MonsterObject : public GLTFSkinModelObject
{

public:
	MonsterObject();
	virtual ~MonsterObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const override;

};

