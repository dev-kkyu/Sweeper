#include "PlayerObject.h"

#include "NetworkManager.h"		// ���콺 ȸ�� ������ ����

PlayerObject::PlayerObject()
{
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime, uint32_t currentFrame)
{
	// Ű �Է¿� ���� �������� �������� ó��

	GLTFSkinModelObject::update(elapsedTime, currentFrame);
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void PlayerObject::release()
{
}
