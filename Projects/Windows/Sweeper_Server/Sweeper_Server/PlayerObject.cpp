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
	moveSpeed = 5.f;							// 초당 이동속도 5m

	gravity = 25.f;								// 중력을 이것으로 조정해 준다.
	jumpSpeed = glm::sqrt(2.f * gravity * 1.f);	// 최대 높이 1m
	velocity = 0.f;

	collisionRadius = 0.4f;						// 캐릭터 충돌 반지름 조정
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

bool PlayerObject::update(float elapsedTime)
{
	// Room의 Update에서 락 걸어준다
	if (isAttack) {
		auto now_time = std::chrono::steady_clock::now();
		if (now_time > attackBeginTime + std::chrono::milliseconds{ 700 }) {
			isAttack = false;
		}
		else if (now_time > attackBeginTime + std::chrono::milliseconds{ 200 }) {
			// 충돌검사
			auto myPos = getPosition();
			myPos += getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.
			for (auto& m : parentRoom->monsters) {
				auto monPos = m.second->getPosition();
				float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
				if (dist2 <= 1.5f) {	// 충돌
					m.second->onHit(*this);
					isAttack = false;
					std::cout << m.first << ": 몬스터 공격받음" << std::endl;
					break;
				}
			}
		}
	}

	if (!keyState and !runJump)		// Position이 변경되지 않으면 false로 리턴한다.
		return false;

	// 공격중이 아니어야 XZ평면으로 움직일 수 있다. 다만 점프중일땐 움직일 수 있다
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

			// 점프 처리
			if (keyState & KEY_SPACE) {
				if (!runJump) {
					velocity = jumpSpeed;
					runJump = true;
				}
			}

			//move(direction, elapsedTime * moveSpeed);
			// 입력된 방향으로 플레이어 방향을 점차 바꾸면서, 해당 방향으로 전진한다
			rotateAndMoveToDirection(direction, elapsedTime);

			// 이동 후 충돌처리
			// 플레이어끼리
			for (int i = 0; i < 4; ++i) {
				if (i == my_id)
					continue;
				if (Room::isValidSession(parentRoom->sessions[i])) {
					if (isCollide(*parentRoom->sessions[i]->player)) {
						glm::vec3 myPos = getPosition();
						glm::vec3 otherPos = parentRoom->sessions[i]->player->getPosition();
						glm::vec3 dir = myPos - otherPos;
						dir.y = 0.f;
						move(dir, elapsedTime * moveSpeed);	// 움직인 방향과 무관하게, 상대와 나의 방향벡터를 구하면 슬라이딩 벡터가 가능하다
					}
				}
			}
			// 몬스터끼리
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

	// 현재 점프 중이라면
	if (runJump) {
		auto pos = getPosition();
		pos.y += velocity * elapsedTime;	// 현재 속도로 위치 업데이트
		velocity -= gravity * elapsedTime;	// 중력에 의한 속도 감소

		if (pos.y < 0.0f) {		// 땅에 닿으면
			pos.y = 0.0f;		// y 위치를 0으로 재설정
			runJump = false;	// 점프 상태 해제
			velocity = 0.0f;	// 속도를 0으로 재설정
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
	//float radianAngle = glm::acos(glm::clamp(dotProduct, -1.f, 1.f));				// 0 ~ pi (예각으로 나온다)
	float radianAngle = glm::angle(look, dir);				// 0 ~ pi (예각으로 나온다)

	float rotateSign = 1.f;
	if (crossProduct.y < 0.f)								// 시계방향으로 돌지 반시계방향으로 돌지 정해준다
		rotateSign = -1.f;

	float angleOffset = radianAngle / glm::pi<float>();		// 각도에 따라 0.f ~ 1.f
	float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// 기본 값은 0.f ~ 0.5f이고, 최소값은 0.2f

	rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

	move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// 현재 회전 방향에 따른 속도 조절
}
