#include "PlayerObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "includes/glm/gtx/vector_angle.hpp"

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: GameObjectBase{ parentRoom, p_id }
{
	moveSpeed = 5.f;							// �ʴ� �̵��ӵ� 5m

	gravity = 25.f;								// �߷��� �̰����� ������ �ش�.
	jumpSpeed = glm::sqrt(2.f * gravity * 1.f);	// �ִ� ���� 1m
	velocity = 0.f;

	collisionRadius = 0.4f;						// ĳ���� �浹 ������ ����
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

bool PlayerObject::update(float elapsedTime)
{
	// Room�� Update���� �� �ɾ��ش�
	if (isAttack) {
		auto now_time = std::chrono::steady_clock::now();
		if (now_time > attackBeginTime + std::chrono::milliseconds{ 700 }) {
			isAttack = false;
		}
		else if (now_time > attackBeginTime + std::chrono::milliseconds{ 200 }) {
			// �浹�˻�
			auto myPos = getPosition();
			myPos += getLook();	// Į ������ �÷��̾� �������� �������ش�.
			for (auto& m : parentRoom->monsters) {
				auto monPos = m.second->getPosition();
				float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
				if (dist2 <= 1.5f) {	// �浹
					m.second->onHit(*this);
					isAttack = false;
					std::cout << m.first << ": ���� ���ݹ���" << std::endl;
					break;
				}
			}
		}
	}

	if (!keyState and !runJump)		// Position�� ������� ������ false�� �����Ѵ�.
		return false;

	// �������� �ƴϾ�� XZ������� ������ �� �ִ�. �ٸ� �������϶� ������ �� �ִ�
	if (keyState) {
		if (not isFixedState or runJump) {
			//glm::vec3 look = getLook();
			//glm::vec3 right = getRight();

			//glm::vec3 direction{ 0.f };
			//if (keyState & KEY_UP) direction += 1.f * look;
			//if (keyState & KEY_DOWN) direction -= 1.f * look;
			//if (keyState & KEY_LEFT) direction += 1.f * right;
			//if (keyState & KEY_RIGHT) direction -= 1.f * right;
			glm::vec3 direction{ 0.f };
			if (keyState & KEY_UP) direction.z += 1.f;
			if (keyState & KEY_DOWN) direction.z -= 1.f;
			if (keyState & KEY_LEFT) direction.x += 1.f;
			if (keyState & KEY_RIGHT) direction.x -= 1.f;

			// ���� ó��
			if (keyState & KEY_SPACE) {
				if (!runJump) {
					velocity = jumpSpeed;
					runJump = true;
				}
			}

			//move(direction, elapsedTime * moveSpeed);
			// �Էµ� �������� �÷��̾� ������ ���� �ٲٸ鼭, �ش� �������� �����Ѵ�
			rotateAndMoveToDirection(direction, elapsedTime);

			// �̵� �� �浹ó��
			// �÷��̾��
			for (int i = 0; i < 4; ++i) {
				if (i == my_id)
					continue;
				if (Room::isValidSession(parentRoom->sessions[i])) {
					if (isCollide(*parentRoom->sessions[i]->player)) {
						glm::vec3 myPos = getPosition();
						glm::vec3 otherPos = parentRoom->sessions[i]->player->getPosition();
						glm::vec3 dir = myPos - otherPos;
						dir.y = 0.f;
						move(dir, elapsedTime * moveSpeed);	// ������ ����� �����ϰ�, ���� ���� ���⺤�͸� ���ϸ� �����̵� ���Ͱ� �����ϴ�
					}
				}
			}
			// ���ͳ���
			for (auto& m : parentRoom->monsters) {
				if (isCollide(*m.second)) {
					glm::vec3 myPos = getPosition();
					glm::vec3 otherPos = m.second->getPosition();
					glm::vec3 dir = myPos - otherPos;
					dir.y = 0.f;
					move(dir, elapsedTime * moveSpeed);
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

void PlayerObject::onHit(const GameObjectBase& other)
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

void PlayerObject::setAttackStart()
{
	isAttack = true;
	attackBeginTime = std::chrono::steady_clock::now();
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

void PlayerObject::rotateAndMoveToDirection(const glm::vec3& direction, float elapsedTime)
{
	if (glm::length(direction) <= 0.f)
		return;

	glm::vec3 look = getLook();
	glm::vec3 dir = glm::normalize(direction);
	glm::vec3 crossProduct = glm::cross(look, dir);
	//float dotProduct = glm::dot(look, dir);
	//float radianAngle = glm::acos(glm::clamp(dotProduct, -1.f, 1.f));				// 0 ~ pi (�������� ���´�)
	float radianAngle = glm::angle(look, dir);				// 0 ~ pi (�������� ���´�)

	float rotateSign = 1.f;
	if (crossProduct.y < 0.f)								// �ð�������� ���� �ݽð�������� ���� �����ش�
		rotateSign = -1.f;

	float angleOffset = radianAngle / glm::pi<float>();		// ������ ���� 0.f ~ 1.f
	float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// �⺻ ���� 0.f ~ 0.5f�̰�, �ּҰ��� 0.2f

	rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

	move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// ���� ȸ�� ���⿡ ���� �ӵ� ����
}
