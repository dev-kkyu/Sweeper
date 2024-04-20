#include "PlayerObject.h"

#include "Room.h"
#include "Session.h"

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: parentRoom{ parentRoom }, my_id{ p_id }
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

bool PlayerObject::update(float elapsedTime)
{
	if (!keyState and !runJump)		// ������Ʈ �� ���� ������ �����Ѵ�.
		return false;

	// �������� �ƴϾ�� XZ������� ������ �� �ִ�. �ٸ� �������϶� ������ �� �ִ�
	if (keyState) {
		if (not isFixedState or runJump) {
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

			move(direction, elapsedTime * 4.f);		// �ʴ� �̵��ӵ� 4m

			// �̵� �� �浹ó��
			// �÷��̾��
			for (int i = 0; i < 4; ++i) {
				if (i == my_id)
					continue;
				if (parentRoom->sessions[i]) {
					if (isCollide(*parentRoom->sessions[i]->player)) {
						glm::vec3 myPos = getPosition();
						glm::vec3 otherPos = parentRoom->sessions[i]->player->getPosition();
						glm::vec3 dir = myPos - otherPos;
						move(dir, elapsedTime * 4.f);	// ������ ����� �����ϰ�, ���� ���� ���⺤�͸� ���ϸ� �����̵� ���Ͱ� �����ϴ�
					}
				}
			}
		}
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

	return true;
}

void PlayerObject::release()
{
}

unsigned int PlayerObject::getKeyState() const
{
	return keyState;
}

bool PlayerObject::getFixedState() const
{
	return isFixedState;
}

void PlayerObject::setFixedState(bool state)
{
	isFixedState = state;
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
