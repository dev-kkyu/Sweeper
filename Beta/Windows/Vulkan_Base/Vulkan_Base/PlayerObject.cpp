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

void PlayerObject::update(float elapsedTime)
{
	if (keyState) {
		glm::vec3 look = getLook();
		glm::vec3 right = getRight();

		glm::vec3 direction{ 0.f };
		if (keyState & KEY_UP) direction += 1.f * look;
		if (keyState & KEY_DOWN) direction -= 1.f * look;
		if (keyState & KEY_LEFT) direction += 1.f * right;
		if (keyState & KEY_RIGHT) direction -= 1.f * right;

		move(direction, elapsedTime * 2.f);
	}
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	GameObject::draw(commandBuffer, pipelineLayout);
}

void PlayerObject::release()
{
}

void PlayerObject::processKeyInput(unsigned int keyState)
{
	this->keyState = keyState;
}
