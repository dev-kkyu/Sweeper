#include "WarriorObject.h"

#include "Room.h"
#include "MonsterObject.h"
#include <iostream>

WarriorATTACKState::WarriorATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void WarriorATTACKState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void WarriorATTACKState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 400 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.

		// 몬스터와 충돌 검사
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();

			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			float targetDist2 = glm::pow(m.second->getCollisionRadius(), 2.f);
			if (dist2 <= targetDist2) {	// 충돌
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
		// 보스와 충돌 검사
		if (0 == attackedObject.count(player.parentRoom->boss.get())) {	// 이전에 공격이 없었어야 한다
			auto monPos = player.parentRoom->boss->getPosition();

			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			float targetDist2 = glm::pow(player.parentRoom->boss->getCollisionRadius(), 2.f);
			if (dist2 <= targetDist2) {	// 충돌
				player.parentRoom->boss->onHit(player, 100);
				attackedObject.insert(player.parentRoom->boss.get());			// 한번만 공격이 들어가도록 한다
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void WarriorATTACKState::exit()
{
	StateMachine::exit();
}

WarriorSKILLState::WarriorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void WarriorSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void WarriorSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 750 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 550 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.

		// 몬스터와 충돌 검사
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();

			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			float targetDist2 = glm::pow(m.second->getCollisionRadius(), 2.f);
			if (dist2 <= targetDist2) {	// 충돌
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
		// 보스와 충돌 검사
		if (0 == attackedObject.count(player.parentRoom->boss.get())) {	// 이전에 공격이 없었어야 한다
			auto monPos = player.parentRoom->boss->getPosition();

			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			float targetDist2 = glm::pow(player.parentRoom->boss->getCollisionRadius(), 2.f);
			if (dist2 <= targetDist2) {	// 충돌
				player.parentRoom->boss->onHit(player, 100);
				attackedObject.insert(player.parentRoom->boss.get());			// 한번만 공격이 들어가도록 한다
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void WarriorSKILLState::exit()
{
	StateMachine::exit();
}

WarriorObject::WarriorObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

void WarriorObject::initialize()
{
}

bool WarriorObject::update(float elapsedTime)
{
	return PlayerObject::update(elapsedTime);
}

void WarriorObject::release()
{
}

void WarriorObject::onHit(const GameObjectBase& other, int damage)
{
}

void WarriorObject::changeATTACKState()
{
	nextState = std::make_unique<WarriorATTACKState>(*this);
}

void WarriorObject::changeSKILLState()
{
	nextState = std::make_unique<WarriorSKILLState>(*this);
}
