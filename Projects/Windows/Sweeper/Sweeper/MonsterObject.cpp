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

	// �̵� ����
	if (MONSTER_CLIP_MOVE == activeAnimation) {
		moveForward(2.f * elapsedTime);		// Todo : ������ ���� ���� �ʿ� (�̵� �ӵ�)
	}
}

void MonsterObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void MonsterObject::release()
{
}
