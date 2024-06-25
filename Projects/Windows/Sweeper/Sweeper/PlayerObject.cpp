#include "PlayerObject.h"

#include "NetworkManager.h"		// KEY EVENT Define�� �������� ����

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

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

	// Ŭ���̾�Ʈ���� �÷��̾� �̵� ����
	glm::vec3 direction{};
	if (keyState & KEY_UP) direction.z += 1.f;
	if (keyState & KEY_DOWN) direction.z -= 1.f;
	if (keyState & KEY_LEFT) direction.x += 1.f;
	if (keyState & KEY_RIGHT) direction.x -= 1.f;
	if (glm::length(direction) >= glm::epsilon<float>()) {
		glm::vec3 look = getLook();
		glm::vec3 dir = glm::normalize(direction);
		glm::vec3 crossProduct = glm::cross(look, dir);
		float radianAngle = glm::angle(look, dir);				// 0 ~ pi (�������� ���´�)

		float rotateSign = 1.f;
		if (crossProduct.y < 0.f)								// �ð�������� ���� �ݽð�������� ���� �����ش�
			rotateSign = -1.f;

		float angleOffset = radianAngle / glm::pi<float>();		// ������ ���� 0.f ~ 1.f
		float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// �⺻ ���� 0.f ~ 0.5f�̰�, �ּҰ��� 0.2f

		rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

		move(dir, elapsedTime * PLAYER_SPEED * (1.f - angleOffset));	// ���� ȸ�� ���⿡ ���� �ӵ� ����
	}
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void PlayerObject::release()
{
}

void PlayerObject::processKeyInput(unsigned int key, bool is_pressed)
{
	if (is_pressed) {
		keyState |= key;
	}
	else {
		keyState &= ~key;
	}
}
