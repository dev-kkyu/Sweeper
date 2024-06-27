#include "PlayerObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "includes/glm/gtx/vector_angle.hpp"

StateMachine::StateMachine(PlayerObject& player)
	: player{ player }
{
}

void StateMachine::enter()
{
	SC_PLAYER_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_PLAYER_STATE;
	p.player_id = player.my_id;
	p.state = state;
	// player update에서 호출되기 때문에, 락이 걸려있다.
	for (auto& s : player.parentRoom->sessions) {			// 모든 플레이어에게 변경된 플레이어 State를 보내준다.
		if (Room::isValidSession(s))
			s->sendPacket(&p);
	}
}

void StateMachine::update(float elapsedTime)
{
	// 점프는 상태 관계없이 해도 된다.

	// 현재 점프 중이라면
	if (player.runJump) {
		auto pos = player.getPosition();
		pos.y += player.velocity * elapsedTime;	// 현재 속도로 위치 업데이트
		player.velocity -= player.gravity * elapsedTime;	// 중력에 의한 속도 감소

		if (pos.y < 0.0f) {		// 땅에 닿으면
			pos.y = 0.0f;		// y 위치를 0으로 재설정
			player.runJump = false;	// 점프 상태 해제
			player.velocity = 0.0f;	// 속도를 0으로 재설정
		}

		player.setPosition(pos);
	}
}

void StateMachine::exit() const
{
}

PLAYER_STATE StateMachine::getState() const
{
	return state;
}

IDLEState::IDLEState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::IDLE;
}

void IDLEState::enter()
{
	StateMachine::enter();
}

void IDLEState::update(float elapsedTime)
{
	// IDLE인 상태에서 할 일..
	// 점프 상태 확인
	if (player.keyState & KEY_SPACE) {
		if (!player.runJump) {
			player.velocity = player.jumpSpeed;
			player.runJump = true;
		}
	}
	// 공격 혹은 이동
	if (player.keyState & MOUSE_LEFT) {
		player.nextState = std::make_unique<AttackState>(player);
	}
	else if (player.keyState & KEY_UP or player.keyState & KEY_DOWN
		or player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT) {
		player.nextState = std::make_unique<RUNState>(player);
	}

	StateMachine::update(elapsedTime);
}

RUNState::RUNState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::RUN;
}

void RUNState::enter()
{
	StateMachine::enter();
}

void RUNState::update(float elapsedTime)
{
	if (player.keyState & MOUSE_LEFT) {
		player.nextState = std::make_unique<AttackState>(player);
	}
	else if (player.keyState & KEY_UP or player.keyState & KEY_DOWN
		or player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT) {

		glm::vec3 direction{ 0.f };
		if (player.keyState & KEY_UP) direction.z += 1.f;
		if (player.keyState & KEY_DOWN) direction.z -= 1.f;
		if (player.keyState & KEY_LEFT) direction.x += 1.f;
		if (player.keyState & KEY_RIGHT) direction.x -= 1.f;

		// 점프 처리
		if (player.keyState & KEY_SPACE) {
			if (!player.runJump) {
				player.velocity = player.jumpSpeed;
				player.runJump = true;
			}
		}

		//move(direction, elapsedTime * moveSpeed);
		// 입력된 방향으로 플레이어 방향을 점차 바꾸면서, 해당 방향으로 전진한다
		player.rotateAndMoveToDirection(direction, elapsedTime);

		// 이동 후 충돌처리
		// 플레이어끼리
		for (int i = 0; i < 4; ++i) {
			if (i == player.my_id)
				continue;
			if (Room::isValidSession(player.parentRoom->sessions[i])) {
				if (player.isCollide(*player.parentRoom->sessions[i]->player)) {
					glm::vec3 myPos = player.getPosition();
					glm::vec3 otherPos = player.parentRoom->sessions[i]->player->getPosition();
					glm::vec3 dir = myPos - otherPos;
					dir.y = 0.f;
					player.move(dir, elapsedTime * player.moveSpeed);	// 움직인 방향과 무관하게, 상대와 나의 방향벡터를 구하면 슬라이딩 벡터가 가능하다
				}
			}
		}
		// 몬스터끼리
		for (auto& m : player.parentRoom->monsters) {
			if (player.isCollide(*m.second)) {
				glm::vec3 myPos = player.getPosition();
				glm::vec3 otherPos = m.second->getPosition();
				glm::vec3 dir = myPos - otherPos;
				dir.y = 0.f;
				player.move(dir, elapsedTime * player.moveSpeed);
			}
		}
	}
	else {		// 더 이상 키 입력이 없다면 IDLE
		player.nextState = std::make_unique<IDLEState>(player);
	}

	StateMachine::update(elapsedTime);
}

DASHState::DASHState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::DASH;
}

void DASHState::enter()
{
	StateMachine::enter();
}

void DASHState::update(float elapsedTime)
{
	// Todo..

	StateMachine::update(elapsedTime);
}

AttackState::AttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void AttackState::enter()
{
	StateMachine::enter();
	attackBeginTime = std::chrono::steady_clock::now();
}

void AttackState::update(float elapsedTime)
{
	// Room의 Update에서 락 걸어준다
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > attackBeginTime + std::chrono::milliseconds{ 700 }) {
		// 끝났으면 State 변경
		player.nextState = std::make_unique<IDLEState>(player);
	}
	else if (now_time > attackBeginTime + std::chrono::milliseconds{ 200 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// 충돌
				m.second->onHit(player);
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: GameObjectBase{ parentRoom, p_id }
{
	currentState = std::make_unique<IDLEState>(*this);

	moveSpeed = PLAYER_SPEED;							// 초당 이동속도 5m

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
	if (nextState) {
		currentState->exit();
		currentState = std::move(nextState);
		currentState->enter();
	}
	currentState->update(elapsedTime);

	if (dynamic_cast<RUNState*>(currentState.get())) {
		return true;	// Run 상태일 때만 위치가 바뀐다.
	}
	return false;
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
