#include "MageObject.h"

#include "Room.h"
#include "MonsterObject.h"
#include <iostream>

MageAttackState::MageAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void MageAttackState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void MageAttackState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 800 }) {
		// �������� State ����
		player.changeIDLEState();
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

void MageAttackState::exit()
{
	StateMachine::exit();
}

MageSKILLState::MageSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
}

void MageSKILLState::enter()
{
}

void MageSKILLState::update(float elapsedTime)
{
}

void MageSKILLState::exit()
{
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

void MageObject::onHit(const GameObjectBase& other)
{
}

void MageObject::changeATTACKState()
{
	nextState = std::make_unique<MageAttackState>(*this);
}

void MageObject::changeSKILLState()
{
	nextState = std::make_unique<MageSKILLState>(*this);
}
