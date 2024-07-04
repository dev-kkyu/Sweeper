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
	// player update���� ȣ��Ǳ� ������, ���� �ɷ��ִ�.
	for (auto& s : player.parentRoom->sessions) {			// ��� �÷��̾�� ����� �÷��̾� State�� �����ش�.
		if (Room::isValidSession(s))
			s->sendPacket(&p);
	}
}

void StateMachine::update(float elapsedTime)
{
	// ������ ���� ������� �ص� �ȴ�.
	if (player.keyState & KEY_SPACE) {
		if (!player.runJump) {
			player.velocity = player.jumpSpeed;
			player.runJump = true;
		}
	}

	// ���� ���� ���̶��
	if (player.runJump) {
		auto pos = player.getPosition();
		pos.y += player.velocity * elapsedTime;	// ���� �ӵ��� ��ġ ������Ʈ
		player.velocity -= player.gravity * elapsedTime;	// �߷¿� ���� �ӵ� ����

		if (pos.y < 0.0f) {		// ���� ������
			pos.y = 0.0f;		// y ��ġ�� 0���� �缳��
			player.runJump = false;	// ���� ���� ����
			player.velocity = 0.0f;	// �ӵ��� 0���� �缳��
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
	// IDLE�� ���¿��� �� ��..

	// ���� Ȥ�� �̵�
	if (player.keyState & MOUSE_LEFT) {
		player.nextState = std::make_unique<AttackState>(player);
	}
	else if (player.keyState & KEY_UP or player.keyState & KEY_DOWN
		or player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT) {
		player.nextState = std::make_unique<RUNState>(player);
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

	maxMoveSpeed = PLAYER_SPEED;				// �ʴ� �̵��ӵ� 5m
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
		player.nextState = std::make_unique<AttackState>(player);
	}
	else if (player.keyState & KEY_CTRL) {
		player.nextState = std::make_unique<DASHState>(player);
	}
	else {
		bool isKeyOn = player.keyState & KEY_UP or player.keyState & KEY_DOWN or
			player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT;
		if (isKeyOn) {
			// ���ӵ� ����
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
		else {	// Ű�� ������ ��
			moveSpeed -= acceleration * elapsedTime;
			if (moveSpeed < 0.f) {
				moveSpeed = 0.f;
				// Ű�� ������ �ӵ��� 0�� �Ǹ� IDLE�� �ٲ��.
				player.nextState = std::make_unique<IDLEState>(player);
			}
		}

		//move(direction, elapsedTime * moveSpeed);
		// �Էµ� �������� �÷��̾� ������ ���� �ٲٸ鼭, �ش� �������� �����Ѵ�
		// �̵��ϸ� �ʰ� �浹üũ�� �Ͽ� �� �� �ִ� ���� ����
		player.moveAndCheckCollision(lastDirection, moveSpeed, elapsedTime);

		// �̵� �� �浹ó��
		// �÷��̾��
		for (int i = 0; i < 4; ++i) {
			if (i == player.my_id)
				continue;
			if (Room::isValidSession(player.parentRoom->sessions[i])) {
				if (player.isCollide(*player.parentRoom->sessions[i]->player)) {
					glm::vec3 myPos = player.getPosition();
					glm::vec3 otherPos = player.parentRoom->sessions[i]->player->getPosition();
					glm::vec3 dir = myPos - otherPos;
					dir.y = 0.f;
					player.move(dir, elapsedTime * moveSpeed);	// ������ ����� �����ϰ�, ���� ���� ���⺤�͸� ���ϸ� �����̵� ���Ͱ� �����ϴ�
				}
			}
		}
		// ���ͳ���
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
		player.nextState = std::make_unique<IDLEState>(player);
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

AttackState::AttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void AttackState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void AttackState::update(float elapsedTime)
{
	// Room�� Update���� �� �ɾ��ش�
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 700 }) {
		// �������� State ����
		player.nextState = std::make_unique<IDLEState>(player);
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// �浹�˻�
		auto myPos = player.getPosition();
		myPos += player.getLook();	// Į ������ �÷��̾� �������� �������ش�.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// �浹
				m.second->onHit(player);
				std::cout << m.first << ": ���� ���ݹ���" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void AttackState::exit()
{
	StateMachine::exit();
}

PlayerObject::PlayerObject(Room* parentRoom, int p_id)
	: GameObjectBase{ parentRoom, p_id }
{
	currentState = std::make_unique<IDLEState>(*this);

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
	if (nextState) {
		currentState->exit();
		currentState = std::move(nextState);
		currentState->enter();
	}
	currentState->update(elapsedTime);

	if (dynamic_cast<RUNState*>(currentState.get()) or dynamic_cast<DASHState*>(currentState.get()) or runJump) {
		return true;	// Run, Dash �����̰ų� ���� ���� ���� ��ġ�� �ٲ��.
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

void PlayerObject::rotateAndMoveToDirection(const glm::vec3& direction, float moveSpeed, float elapsedTime)
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

void PlayerObject::moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime)
{
	auto befPos = getPosition();
	rotateAndMoveToDirection(direction, moveSpeed, elapsedTime);
	auto& boundBox = Map::getInstance().getBoundingBox();
	for (const auto& box : boundBox) {
		if (box.isCollide(getBoundingBox())) {
			setPosition(befPos);			// �浹�� ���� ���� ��ġ�� ���ư���.
			// �����̵� ���� ����
			glm::vec3 pDir = getLook();	// ���� �����ִ� ����
			pDir.y = 0.f;
			glm::vec3 moveDir = pDir;			// �̵��� ����
			auto pBox = getBoundingBox();// �浹 ���� �ٿ�� �ڽ�
			// ���� �������� ��� ������ �浹�̾����� Ȯ�� (�浹 �� ��ġ�� ���ư�����, �ش� ���⿡���� ���� �浹�� �ƴϴ�)
			glm::vec3 newPos = befPos;
			if (box.getBack() > pBox.getFront() or box.getFront() < pBox.getBack()) {
				moveDir.z = 0.f;
				if (box.getBack() > pBox.getFront()) {	// �÷��̾ �ڽ��� ����
					newPos.z = box.getBack() - getCollisionRadius() - 0.00001f;
				}
				else {									// �÷��̾ �ڽ��� ����
					newPos.z = box.getFront() + getCollisionRadius() + 0.00001f;
				}
			}
			if (box.getLeft() > pBox.getRight() or box.getRight() < pBox.getLeft()) {
				moveDir.x = 0.f;
				if (box.getLeft() > pBox.getRight()) {	// �÷��̾ �ڽ��� ����
					newPos.x = box.getLeft() - getCollisionRadius() - 0.00001f;
				}
				else {									// �÷��̾ �ڽ��� ������
					newPos.x = box.getRight() + getCollisionRadius() + 0.00001f;
				}
			}
			setPosition(newPos);			// �浹�� �ƴ� �ִ��� ��ġ�� ������ �ش�.
			if (glm::length(moveDir) > 0.f) {
				// �������� �ι踦 �Ͽ� ������ ���� �����ϰ� �ι��� �ӵ�, �ִ� 1.f�� �ӵ��� �����̵��� �Ѵ�.
				float moveOffset = glm::clamp(glm::dot(pDir, glm::normalize(moveDir)) * 2.f, 0.f, 1.f);
				move(moveDir, moveOffset * moveSpeed * elapsedTime);
				// �����̵��� �Ͽ������� �浹�̸� ���� ��ġ�� ���ư���, ���̻� �������� �ʴ´�.
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
