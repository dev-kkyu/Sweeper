#include "HealerObject.h"

#include "Room.h"
#include "MonsterObject.h"
#include <iostream>

HealerAttackState::HealerAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void HealerAttackState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void HealerAttackState::update(float elapsedTime)
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

void HealerAttackState::exit()
{
	StateMachine::exit();
}

HealerSKILLState::HealerSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
}

void HealerSKILLState::enter()
{
}

void HealerSKILLState::update(float elapsedTime)
{
}

void HealerSKILLState::exit()
{
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

void HealerObject::onHit(const GameObjectBase& other)
{
}

void HealerObject::changeATTACKState()
{
	nextState = std::make_unique<HealerAttackState>(*this);
}

void HealerObject::changeSKILLState()
{
	nextState = std::make_unique<HealerSKILLState>(*this);
}
