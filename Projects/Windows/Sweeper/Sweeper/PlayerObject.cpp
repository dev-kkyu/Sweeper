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
