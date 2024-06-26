#include "MonsterObject.h"

MonsterObject::MonsterObject()
{
}

MonsterObject::~MonsterObject()
{
}

void MonsterObject::initialize()
{
}

void MonsterObject::update(float elapsedTime, uint32_t currentFrame)
{
	GLTFSkinModelObject::update(elapsedTime, currentFrame);

	// 이동 보정
	if (MONSTER_CLIP_MOVE == activeAnimation) {
		moveForward(2.f * elapsedTime);		// Todo : 서버와 공유 변수 필요 (이동 속도)
	}
}

void MonsterObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void MonsterObject::release()
{
}
