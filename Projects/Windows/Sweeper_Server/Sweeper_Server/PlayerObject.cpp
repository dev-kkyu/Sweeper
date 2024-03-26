#include "PlayerObject.h"

PlayerObject::PlayerObject()
{
	gravity = 25.f;								// �߷��� �̰����� ������ �ش�.
	jumpSpeed = glm::sqrt(2.f * gravity * 1.f);	// �ִ� ���� 1m
	velocity = 0.f;
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime)
{
	if (!keyState and !runJump)		// ������Ʈ �� ���� ������ �����Ѵ�.
		return;

	if (keyState) {
		glm::vec3 look = getLook();
		glm::vec3 right = getRight();

		glm::vec3 direction{ 0.f };
		if (keyState & KEY_UP) direction += 1.f * look;
		if (keyState & KEY_DOWN) direction -= 1.f * look;
		if (keyState & KEY_LEFT) direction += 1.f * right;
		if (keyState & KEY_RIGHT) direction -= 1.f * right;

		// ���� ó��
		if (keyState & KEY_SPACE) {
			if (!runJump) {
				velocity = jumpSpeed;
				runJump = true;
			}
		}

		move(direction, elapsedTime * 4.f);
	}

	// ���� ���� ���̶��
	if (runJump) {
		auto pos = getPosition();
		pos.y += velocity * elapsedTime;	// ���� �ӵ��� ��ġ ������Ʈ
		velocity -= gravity * elapsedTime;	// �߷¿� ���� �ӵ� ����

		if (pos.y < 0.0f) {		// ���� ������
			pos.y = 0.0f;		// y ��ġ�� 0���� �缳��
			runJump = false;	// ���� ���� ����
			velocity = 0.0f;	// �ӵ��� 0���� �缳��
		}

		setPosition(pos);
	}
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

void PlayerObject::processMoveMouse(float move_x, float move_y)
{
	rotate(-move_x * 100.f);
}
