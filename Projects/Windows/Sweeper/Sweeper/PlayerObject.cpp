#include "PlayerObject.h"

#include "NetworkManager.h"		// KEY EVENT Define�� �������� ����

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

PlayerObject::PlayerObject()
{
	maxMoveSpeed = PLAYER_SPEED;				// �ʴ� �̵��ӵ� 5m
	moveSpeed = 0.f;
	acceleration = 25.f;
	lastDirection = glm::vec3{};

	dashFlag = true;
	dashAccFlag = 1;							// Todo : �� �������� ������ ����ȭ �ʿ�
	dashMaxMoveSpeed = 30.f;
	dashMoveSpeed = 0.f;
	dashAcceleration = 250.f;

	collisionRadius = 0.4f;						// ĳ���� �浹 ������ ����
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
	if (keyState & MOUSE_LEFT or (dashFlag and keyState & KEY_CTRL)) {
		moveSpeed = 0.f;
		if (keyState & KEY_CTRL) {
			dashMoveSpeed = 0.f;
			dashAccFlag = 1;
			dashFlag = false;	// Ctrl�� ó�� ������ �� �ʱ�ȭ
		}
	}
	else if (PLAYER_CLIP_DASH == activeAnimation) {		// Dash �̵� ����
		if (dashAccFlag > 0) {
			dashMoveSpeed += dashAcceleration * elapsedTime;
			if (dashMoveSpeed >= dashMaxMoveSpeed) {
				dashMoveSpeed = dashMaxMoveSpeed;
				dashAccFlag = -1;
			}
		}
		else {
			dashMoveSpeed -= dashAcceleration * elapsedTime;
			if (dashMoveSpeed < 0.f)
				dashMoveSpeed = 0.f;
		}
		if (glm::length(lastDirection) >= glm::epsilon<float>()) {
			glm::vec3 look = getLook();
			glm::vec3 dir = glm::normalize(lastDirection);
			glm::vec3 crossProduct = glm::cross(look, dir);
			float radianAngle = glm::angle(look, dir);				// 0 ~ pi (�������� ���´�)

			float rotateSign = 1.f;
			if (crossProduct.y < 0.f)								// �ð�������� ���� �ݽð�������� ���� �����ش�
				rotateSign = -1.f;

			float angleOffset = radianAngle / glm::pi<float>();		// ������ ���� 0.f ~ 1.f
			float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// �⺻ ���� 0.f ~ 0.5f�̰�, �ּҰ��� 0.2f

			rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

			move(dir, elapsedTime * dashMoveSpeed * (1.f - angleOffset));	// ���� ȸ�� ���⿡ ���� �ӵ� ����
		}
	}
	else {
		if (PLAYER_CLIP_RUN == activeAnimation) {	// RUN �̵� ����
			bool isKeyOn = keyState & KEY_UP or keyState & KEY_DOWN or
				keyState & KEY_LEFT or keyState & KEY_RIGHT;
			if (isKeyOn) {
				// ���ӵ� ����
				moveSpeed += acceleration * elapsedTime;
				if (moveSpeed > maxMoveSpeed)
					moveSpeed = maxMoveSpeed;

				glm::vec3 direction{ 0.f };
				if (keyState & KEY_UP) direction.z += 1.f;
				if (keyState & KEY_DOWN) direction.z -= 1.f;
				if (keyState & KEY_LEFT) direction.x += 1.f;
				if (keyState & KEY_RIGHT) direction.x -= 1.f;

				lastDirection = direction;
			}
			else {	// Ű�� ������ ��
				moveSpeed -= acceleration * elapsedTime;
				if (moveSpeed < 0.f) {
					moveSpeed = 0.f;
				}
			}

			if (glm::length(lastDirection) >= glm::epsilon<float>()) {
				glm::vec3 look = getLook();
				glm::vec3 dir = glm::normalize(lastDirection);
				glm::vec3 crossProduct = glm::cross(look, dir);
				float radianAngle = glm::angle(look, dir);				// 0 ~ pi (�������� ���´�)

				float rotateSign = 1.f;
				if (crossProduct.y < 0.f)								// �ð�������� ���� �ݽð�������� ���� �����ش�
					rotateSign = -1.f;

				float angleOffset = radianAngle / glm::pi<float>();		// ������ ���� 0.f ~ 1.f
				float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// �⺻ ���� 0.f ~ 0.5f�̰�, �ּҰ��� 0.2f

				rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

				move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// ���� ȸ�� ���⿡ ���� �ӵ� ����
			}
		}
		// Dash Animation�� �ƴ� �� true
		dashFlag = true;
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
