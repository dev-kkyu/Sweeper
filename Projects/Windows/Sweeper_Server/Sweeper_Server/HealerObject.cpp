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
		// �������� State ����
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// �浹�˻�
		auto myPos = player.getPosition();
		myPos += player.getLook();	// Į ������ �÷��̾� �������� �������ش�.
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// �̹� ������ ������ ������ �Ѿ��
				continue;
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// �浹
				m.second->onHit(player, 100);
				attackedObject.insert(m.second.get());			// �ѹ��� ������ ������ �Ѵ�
				std::cout << m.first << ": ���� ���ݹ���" << std::endl;
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
		// �������� State ����
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// ���� ü�� ȸ��
		glm::vec3 spawnPos = player.getPosition() + player.getLook() * 5.5f;		// ��ų ���� ����
		for (int i = 0; i < player.parentRoom->sessions.size(); ++i) {				// ������ ���� �� �� �ִ�.
			std::shared_ptr<Session> session = player.parentRoom->sessions[i].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				float dist2 = glm::pow(spawnPos.x - playerPos.x, 2.f) + glm::pow(spawnPos.z - playerPos.z, 2.f);
				float range2 = glm::pow(3.f, 2.f);		// ������ 3 ���� ������ �ִ���
				if (dist2 <= range2) {
					// Todo ü�� ȸ��, �� �ڵ带 player�� update�� �Űܾ� �Ѵ�
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
