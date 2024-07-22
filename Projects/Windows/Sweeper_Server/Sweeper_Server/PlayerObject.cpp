#include "PlayerObject.h"

#include "Map.h"
#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

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
	for (auto& a : player.parentRoom->sessions) {			// 모든 플레이어에게 변경된 플레이어 State를 보내준다.
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session))
			session->sendPacket(&p);
	}
}

void StateMachine::update(float elapsedTime)
{
	// 점프는 상태 관계없이 해도 된다.
	if (player.keyState & KEY_SPACE) {
		if (!player.runJump) {
			player.velocity = player.jumpSpeed;
			player.runJump = true;
		}
	}

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

void StateMachine::exit()
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

	// 공격 혹은 이동
	if (player.keyState & MOUSE_LEFT) {
		player.changeATTACKState();
	}
	else if (player.keyState & MOUSE_RIGHT) {
		player.changeSKILLState();
	}
	else if (player.keyState & KEY_UP or player.keyState & KEY_DOWN
		or player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT) {
		player.changeRUNState();
	}

	StateMachine::update(elapsedTime);
}

void IDLEState::exit()
{
	StateMachine::exit();
}

RUNState::RUNState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::RUN;

	maxMoveSpeed = PLAYER_SPEED;				// 초당 이동속도 5m
	moveSpeed = 0.f;
	acceleration = 25.f;

	lastDirection = glm::vec3{ 0.f };
}

void RUNState::enter()
{
	StateMachine::enter();
}

void RUNState::update(float elapsedTime)
{
	if (player.keyState & MOUSE_LEFT) {
		player.changeATTACKState();
	}
	else if (player.keyState & MOUSE_RIGHT) {
		player.changeSKILLState();
	}
	else if (player.keyState & KEY_CTRL) {
		player.changeDASHState();
	}
	else {
		bool isKeyOn = player.keyState & KEY_UP or player.keyState & KEY_DOWN or
			player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT;
		if (isKeyOn) {
			// 가속도 적용
			moveSpeed += acceleration * elapsedTime;
			if (moveSpeed > maxMoveSpeed)
				moveSpeed = maxMoveSpeed;

			glm::vec3 direction{ 0.f };
			if (player.keyState & KEY_UP) direction.z += 1.f;
			if (player.keyState & KEY_DOWN) direction.z -= 1.f;
			if (player.keyState & KEY_LEFT) direction.x += 1.f;
			if (player.keyState & KEY_RIGHT) direction.x -= 1.f;

			lastDirection = direction;
		}
		else {	// 키가 떼졌을 때
			moveSpeed -= acceleration * elapsedTime;
			if (moveSpeed < 0.f) {
				moveSpeed = 0.f;
				// 키가 떼지고 속도가 0이 되면 IDLE로 바뀐다.
				player.changeIDLEState();
			}
		}

		//move(direction, elapsedTime * moveSpeed);
		// 입력된 방향으로 플레이어 방향을 점차 바꾸면서, 해당 방향으로 전진한다
		// 이동하며 맵과 충돌체크를 하여 갈 수 있는 곳만 간다
		player.moveAndCheckCollision(lastDirection, moveSpeed, elapsedTime);

		// 이동 후 충돌처리
		// 플레이어끼리
		for (int i = 0; i < 4; ++i) {
			if (i == player.my_id)
				continue;
			std::shared_ptr<Session> session = player.parentRoom->sessions[i].load();
			if (Room::isValidSession(session)) {
				if (player.isCollide(*session->player)) {
					glm::vec3 myPos = player.getPosition();
					glm::vec3 otherPos = session->player->getPosition();
					glm::vec3 dir = myPos - otherPos;
					dir.y = 0.f;
					player.move(dir, elapsedTime * moveSpeed);	// 움직인 방향과 무관하게, 상대와 나의 방향벡터를 구하면 슬라이딩 벡터가 가능하다
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
				player.move(dir, elapsedTime * moveSpeed);
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void RUNState::exit()
{
	StateMachine::exit();
}

DASHState::DASHState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::DASH;

	direction = glm::vec3{ 0.f };
	if (player.keyState & KEY_UP) direction.z += 1.f;
	if (player.keyState & KEY_DOWN) direction.z -= 1.f;
	if (player.keyState & KEY_LEFT) direction.x += 1.f;
	if (player.keyState & KEY_RIGHT) direction.x -= 1.f;

	accFlag = 1;

	maxMoveSpeed = 30.f;
	moveSpeed = 0.f;
	acceleration = 250.f;
}

void DASHState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void DASHState::update(float elapsedTime)
{
	if (stateBeginTime + std::chrono::milliseconds(400) <= std::chrono::steady_clock::now()) {
		player.changeIDLEState();
	}

	if (accFlag > 0) {
		moveSpeed += acceleration * elapsedTime;
		if (moveSpeed >= maxMoveSpeed) {
			moveSpeed = maxMoveSpeed;
			accFlag = -1;
		}
	}
	else {
		moveSpeed -= acceleration * elapsedTime;
		if (moveSpeed < 0.f)
			moveSpeed = 0.f;
	}
	player.moveAndCheckCollision(direction, moveSpeed, elapsedTime);

	StateMachine::update(elapsedTime);
}

void DASHState::exit()
{
	StateMachine::exit();
}

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: GameObjectBase{ parentRoom, p_id }
{
	currentState = std::make_unique<IDLEState>(*this);

	gravity = 25.f;								// 중력을 이것으로 조정해 준다.
	jumpSpeed = glm::sqrt(2.f * gravity * 1.f);	// 최대 높이 1m
	velocity = 0.f;

	collisionRadius = 0.4f;						// 캐릭터 충돌 반지름 조정
}

void PlayerObject::initialize()
{
}

bool PlayerObject::update(float elapsedTime)
{
	if (nextState) {
		currentState->exit();
		currentState = std::move(nextState);
		currentState->enter();
	}
	currentState->update(elapsedTime);

	if (dynamic_cast<RUNState*>(currentState.get()) or dynamic_cast<DASHState*>(currentState.get()) or runJump) {
		return true;	// Run, Dash 상태이거나 점프 중일 때만 위치가 바뀐다.
	}
	return false;
}

void PlayerObject::release()
{
}

void PlayerObject::onHit(const GameObjectBase& other, int damage)
{
	HP -= damage;

	{	// 브로드캐스트
		SC_PLAYER_HP_PACKET p;
		p.size = sizeof(p);
		p.type = SC_PLAYER_HP;
		p.player_id = my_id;
		p.hp = HP;
		for (auto& a : parentRoom->sessions) {	// 모든 플레이어에게 변경된 몬스터의 Look과 HP를 보내준다.
			std::shared_ptr<Session> session = a.load();
			if (Room::isValidSession(session)) {
				session->sendPacket(&p);
			}
		}
	}
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

void PlayerObject::rotateAndMoveToDirection(const glm::vec3& direction, float moveSpeed, float elapsedTime)
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

void PlayerObject::moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime)
{
	auto befPos = getPosition();
	rotateAndMoveToDirection(direction, moveSpeed, elapsedTime);
	auto& boundBox = Map::getInstance().getBoundingBox();
	for (const auto& box : boundBox) {
		if (box.isCollide(getBoundingBox())) {
			setPosition(befPos);			// 충돌시 먼저 기존 위치로 돌아간다.
			// 슬라이딩 벡터 구현
			glm::vec3 pDir = getLook();	// 현재 보고있는 방향
			pDir.y = 0.f;
			glm::vec3 moveDir = pDir;			// 이동할 방향
			auto pBox = getBoundingBox();// 충돌 전의 바운딩 박스
			// 기존 지점에서 어느 방향이 충돌이었는지 확인 (충돌 전 위치로 돌아갔으니, 해당 방향에서는 현재 충돌이 아니다)
			glm::vec3 newPos = befPos;
			if (box.getBack() > pBox.getFront() or box.getFront() < pBox.getBack()) {
				moveDir.z = 0.f;
				if (box.getBack() > pBox.getFront()) {	// 플레이어가 박스의 뒤쪽
					newPos.z = box.getBack() - getCollisionRadius() - 0.00001f;
				}
				else {									// 플레이어가 박스의 앞쪽
					newPos.z = box.getFront() + getCollisionRadius() + 0.00001f;
				}
			}
			if (box.getLeft() > pBox.getRight() or box.getRight() < pBox.getLeft()) {
				moveDir.x = 0.f;
				if (box.getLeft() > pBox.getRight()) {	// 플레이어가 박스의 왼쪽
					newPos.x = box.getLeft() - getCollisionRadius() - 0.00001f;
				}
				else {									// 플레이어가 박스의 오른쪽
					newPos.x = box.getRight() + getCollisionRadius() + 0.00001f;
				}
			}
			setPosition(newPos);			// 충돌이 아닌 최대의 위치로 움직여 준다.
			if (glm::length(moveDir) > 0.f) {
				// 내적값의 두배를 하여 직각일 때를 제외하고 두배의 속도, 최대 1.f의 속도로 움직이도록 한다.
				float moveOffset = glm::clamp(glm::dot(pDir, glm::normalize(moveDir)) * 2.f, 0.f, 1.f);
				move(moveDir, moveOffset * moveSpeed * elapsedTime);
				// 슬라이딩을 하였음에도 충돌이면 기존 위치로 돌아가고, 더이상 움직이지 않는다.
				for (const auto& box2 : boundBox) {
					if (box2.isCollide(getBoundingBox())) {
						setPosition(newPos);
						break;
					}
				}
			}
			break;
		}
	}
}

void PlayerObject::changeIDLEState()
{
	nextState = std::make_unique<IDLEState>(*this);
}

void PlayerObject::changeRUNState()
{
	nextState = std::make_unique<RUNState>(*this);
}

void PlayerObject::changeDASHState()
{
	nextState = std::make_unique<DASHState>(*this);
}
