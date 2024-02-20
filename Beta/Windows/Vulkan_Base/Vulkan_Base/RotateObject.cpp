#include "RotateObject.h"

RotateObject::RotateObject()
{
}

RotateObject::~RotateObject()
{
}

void RotateObject::initialize()
{
}

void RotateObject::update(float elapsedTime)
{
	// �ʴ� 90�� ȸ��
	rotateAngle += elapsedTime * rotateSpeed;

	auto position = getPosition();
	modelTransform = glm::rotate(glm::mat4(1.f), glm::radians(rotateAngle), glm::vec3(0.f, 1.f, 0.f));
	setPosition(position);
}

void RotateObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	GameObject::draw(commandBuffer, pipelineLayout);
}

void RotateObject::release()
{
}

void RotateObject::setRotateSpeed(float speed)
{
	rotateSpeed = speed;
}
