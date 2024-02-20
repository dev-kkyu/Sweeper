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
		glm::vec3 direction{ 0.f };
		if (keyState & KEY_UP) direction.z -= 1.f;
		if (keyState & KEY_DOWN) direction.z += 1.f;
		if (keyState & KEY_LEFT) direction.x -= 1.f;
		if (keyState & KEY_RIGHT) direction.x += 1.f;

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
