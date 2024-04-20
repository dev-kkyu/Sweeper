#include "PlayerObject.h"

#include "Room.h"
#include "Session.h"

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: parentRoom{ parentRoom }, my_id{ p_id }
{
	gravity = 25.f;								// 중력을 이것으로 조정해 준다.
	jumpSpeed = glm::sqrt(2.f * gravity * 1.f);	// 최대 높이 1m
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
	if (!keyState and !runJump)		// 업데이트 할 것이 없으면 리턴한다.
		return false;

	// 공격중이 아니어야 XZ평면으로 움직일 수 있다. 다만 점프중일땐 움직일 수 있다
	if (keyState) {
		if (not isFixedState or runJump) {
			glm::vec3 look = getLook();
			glm::vec3 right = getRight();

			glm::vec3 direction{ 0.f };
			if (keyState & KEY_UP) direction += 1.f * look;
			if (keyState & KEY_DOWN) direction -= 1.f * look;
			if (keyState & KEY_LEFT) direction += 1.f * right;
			if (keyState & KEY_RIGHT) direction -= 1.f * right;

			// 점프 처리
			if (keyState & KEY_SPACE) {
				if (!runJump) {
					velocity = jumpSpeed;
					runJump = true;
				}
			}

			move(direction, elapsedTime * 4.f);		// 초당 이동속도 4m

			// 이동 후 충돌처리
			// 플레이어끼리
			for (int i = 0; i < 4; ++i) {
				if (i == my_id)
					continue;
				if (parentRoom->sessions[i]) {
					if (isCollide(*parentRoom->sessions[i]->player)) {
						glm::vec3 myPos = getPosition();
						glm::vec3 otherPos = parentRoom->sessions[i]->player->getPosition();
						glm::vec3 dir = myPos - otherPos;
						move(dir, elapsedTime * 4.f);	// 움직인 방향과 무관하게, 상대와 나의 방향벡터를 구하면 슬라이딩 벡터가 가능하다
					}
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
