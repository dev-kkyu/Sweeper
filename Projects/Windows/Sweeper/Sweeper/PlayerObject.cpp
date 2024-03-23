#include "PlayerObject.h"

#include "NetworkManager.h"		// 마우스 회전 전송을 위함

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
	// 키 입력에 따른 움직임은 서버에서 처리

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

	// 서버에서 회전한 뒤, 클라에서 바꿔준다.

	CS_MOVE_MOUSE_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MOVE_MOUSE;
	p.move_x = moveX;
	p.move_y = moveY;
	NetworkManager::getInstance().sendPacket(&p);
}
