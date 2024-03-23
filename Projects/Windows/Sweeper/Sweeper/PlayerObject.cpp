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

void PlayerObject::setStartMousePos(float xpos, float ypos)
{
	startXpos = xpos;
	startYpos = ypos;
}

void PlayerObject::processMouseCursor(float xpos, float ypos)
{
	float moveX = xpos - startXpos;
	float moveY = ypos - startYpos;
	startXpos = xpos;
	startYpos = ypos;

	// �������� ȸ���� ��, Ŭ�󿡼� �ٲ��ش�.

	CS_MOVE_MOUSE_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MOVE_MOUSE;
	p.move_x = moveX;
	p.move_y = moveY;
	NetworkManager::getInstance().sendPacket(&p);
}
