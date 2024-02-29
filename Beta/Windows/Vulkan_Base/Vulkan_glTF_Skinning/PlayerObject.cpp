#include "PlayerObject.h"

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
	if (keyState) {
		glm::vec3 look = getLook();
		glm::vec3 right = getRight();

		glm::vec3 direction{ 0.f };
		if (keyState & KEY_UP) direction += 1.f * look;
		if (keyState & KEY_DOWN) direction -= 1.f * look;
		if (keyState & KEY_LEFT) direction += 1.f * right;
		if (keyState & KEY_RIGHT) direction -= 1.f * right;

		move(direction, elapsedTime * 3.f);
	}
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GameObjectObj::draw(commandBuffer, pipelineLayout, currentFrame);
}

void PlayerObject::release()
{
}

void PlayerObject::processKeyInput(unsigned int keyState)
{
	this->keyState = keyState;
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

	rotate(-moveX * 100.f);
}
