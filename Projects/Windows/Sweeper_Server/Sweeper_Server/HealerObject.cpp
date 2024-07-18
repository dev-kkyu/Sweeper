#include "HealerObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>

HealerATTACKState::HealerATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void HealerATTACKState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void HealerATTACKState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 730 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// 충돌
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void HealerATTACKState::exit()
{
	StateMachine::exit();
}

HealerSKILLState::HealerSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void HealerSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void HealerSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 800 }) {
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// 팀원 체력 회복
		glm::vec3 spawnPos = player.getPosition() + player.getLook() * 5.5f;		// 스킬 범위 지정
		for (int i = 0; i < player.parentRoom->sessions.size(); ++i) {				// 힐러도 힐이 될 수 있다.
			std::shared_ptr<Session> session = player.parentRoom->sessions[i].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				float dist2 = glm::pow(spawnPos.x - playerPos.x, 2.f) + glm::pow(spawnPos.z - playerPos.z, 2.f);
				float range2 = glm::pow(3.f, 2.f);		// 반지름 3 내의 범위에 있는지
				if (dist2 <= range2) {
					// Todo 체력 회복, 이 코드를 player의 update로 옮겨야 한다
				}
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void HealerSKILLState::exit()
{
	StateMachine::exit();
}

HealerObject::HealerObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

void HealerObject::initialize()
{
}

bool HealerObject::update(float elapsedTime)
{
	return PlayerObject::update(elapsedTime);
}

void HealerObject::release()
{
}

void HealerObject::onHit(const GameObjectBase& other, int damage)
{
}

void HealerObject::changeATTACKState()
{
	nextState = std::make_unique<HealerATTACKState>(*this);
}

void HealerObject::changeSKILLState()
{
	nextState = std::make_unique<HealerSKILLState>(*this);
}
