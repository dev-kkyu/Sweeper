#include "MageObject.h"

#include "Room.h"
#include "MonsterObject.h"
#include <iostream>

MageATTACKState::MageATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void MageATTACKState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void MageATTACKState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 800 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 230 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook() * 3.f;	// 마법 공격 중앙
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= (1.f * 1.f)) {	// 공격 범위 중심에서 1.f 거리 내
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void MageATTACKState::exit()
{
	StateMachine::exit();
}

MageSKILLState::MageSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void MageSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void MageSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 700 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 300 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook() * 3.f;	// 마법 공격 중앙
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= (1.5f * 1.5f)) {	// 공격 범위 중심에서 1.5 거리 내
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void MageSKILLState::exit()
{
	StateMachine::exit();
}

MageObject::MageObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

void MageObject::initialize()
{
}

bool MageObject::update(float elapsedTime)
{
	return PlayerObject::update(elapsedTime);
}

void MageObject::release()
{
}

void MageObject::onHit(const GameObjectBase& other, int damage)
{
}

void MageObject::changeATTACKState()
{
	nextState = std::make_unique<MageATTACKState>(*this);
}

void MageObject::changeSKILLState()
{
	nextState = std::make_unique<MageSKILLState>(*this);
}
