#include "ArchorObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>

ArchorAttackState::ArchorAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArchorAttackState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	{
		SC_ADD_ARROW_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_ARROW;
		static int iid;
		p.arrow_id = iid++;
		auto pos = player.getPosition();
		auto dir = player.getLook();
		p.pos_x = pos.x; p.pos_z = pos.z;
		p.dir_x = dir.x; p.dir_z = dir.z;

		// ��� �÷��̾�� ȭ�� �߰� ����� ����
		for (auto& a : player.parentRoom->sessions) {
			std::shared_ptr<Session> session = a.load();
			if (Room::isValidSession(session))
				session->sendPacket(&p);
		}
	}

	StateMachine::enter();
}

void ArchorAttackState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
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

void ArchorAttackState::exit()
{
	StateMachine::exit();
}

ArchorSKILLState::ArchorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArchorSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void ArchorSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
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

void ArchorSKILLState::exit()
{
	StateMachine::exit();
}

ArchorObject::ArchorObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

void ArchorObject::initialize()
{
}

bool ArchorObject::update(float elapsedTime)
{
	return PlayerObject::update(elapsedTime);
}

void ArchorObject::release()
{
}

void ArchorObject::onHit(const GameObjectBase& other)
{
}

void ArchorObject::changeATTACKState()
{
	nextState = std::make_unique<ArchorAttackState>(*this);
}

void ArchorObject::changeSKILLState()
{
	nextState = std::make_unique<ArchorSKILLState>(*this);
}
