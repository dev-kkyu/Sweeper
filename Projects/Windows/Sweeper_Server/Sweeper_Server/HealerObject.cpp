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

		// ���Ϳ� �浹 �˻�
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// �̹� ������ ������ ������ �Ѿ��
				continue;
			auto monPos = m.second->getPosition();

			float dist = glm::sqrt(glm::pow(myPos.x - monPos.x, 2.f) + glm::pow(myPos.z - monPos.z, 2.f));
			float targetDist = 1.f + m.second->getCollisionRadius();		// ���� ���� : �÷��̾� ���� 1.f���� ������ 1.f�� ��
			if (dist <= targetDist) {	// �浹
				m.second->onHit(player, player.attackDamage);
				attackedObject.insert(m.second.get());			// �ѹ��� ������ ������ �Ѵ�
				std::cout << m.first << ": ���� ���ݹ���" << std::endl;
			}
		}
		// ������ �浹 �˻�
		if (0 == attackedObject.count(player.parentRoom->boss.get())) {	// ������ ������ ������� �Ѵ�
			auto monPos = player.parentRoom->boss->getPosition();

			float dist = glm::sqrt(glm::pow(myPos.x - monPos.x, 2.f) + glm::pow(myPos.z - monPos.z, 2.f));
			float targetDist = 1.f + player.parentRoom->boss->getCollisionRadius();		// ���� ���� : �÷��̾� ���� 1.f���� ������ 1.f�� ��
			if (dist <= targetDist) {	// �浹
				player.parentRoom->boss->onHit(player, player.attackDamage);
				attackedObject.insert(player.parentRoom->boss.get());			// �ѹ��� ������ ������ �Ѵ�
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

	dynamic_cast<HealerObject*>(&player)->healerEffects.push_back(HealerObject::HealerEffect{ player.getPosition() + player.getLook() * 5.5f, -0.4f });
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
	maxHP = HP = MAX_HP_PLAYER_HEALER;

	attackDamage = 100;
	skillDamage = 30;		// �÷��̾�� �ִ� ����
}

void HealerObject::initialize()
{
}

bool HealerObject::update(float elapsedTime)
{
	// ����Ʈ �ð� ������Ʈ
	for (auto& hEffect : healerEffects) {
		hEffect.accumTime += elapsedTime;
	}
	// ����Ʈ ���� ����
	std::list<std::list<HealerObject::HealerEffect>::iterator> deleteEffects;
	for (auto itr = healerEffects.begin(); itr != healerEffects.end(); ++itr) {
		if (itr->accumTime >= 2.35f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		healerEffects.erase(itr);
	}
	// ����Ʈ �����Ǵµ��� ��
	for (auto& hEffect : healerEffects) {
		if (hEffect.accumTime >= 0.f) {
			for (auto& a : parentRoom->sessions) {
				std::shared_ptr<Session> session = a.load();
				if (Room::isValidSession(session)) {
					auto playerPos = session->player->getPosition();
					float dist = glm::length(playerPos - hEffect.pos);
					if (dist <= session->player->getCollisionRadius() + 2.f) {
						// ����Ʈ�κ��� ������ 2.f �ȿ� �ִٸ� ��
						PlayerObject* ptrPlayer = session->player.get();
						auto nowTime = std::chrono::steady_clock::now();
						if (hEffect.healedPlayer.find(ptrPlayer) == hEffect.healedPlayer.end()) {
							hEffect.healedPlayer.try_emplace(ptrPlayer, nowTime);
							short nowHP = session->player->getHP();
							session->player->setHP(nowHP + skillDamage);
							session->player->broadcastMyHP();
						}	// 0.75�ʸ��� ��
						else if (hEffect.healedPlayer[ptrPlayer] + std::chrono::milliseconds(750) <= nowTime) {
							hEffect.healedPlayer[ptrPlayer] += std::chrono::milliseconds(750);
							short nowHP = session->player->getHP();
							session->player->setHP(nowHP + skillDamage);
							session->player->broadcastMyHP();
						}
					}
				}
			}
		}
	}

	return PlayerObject::update(elapsedTime);
}

void HealerObject::release()
{
}

void HealerObject::onHit(const GameObjectBase& other, int damage)
{
	PlayerObject::onHit(other, damage);
}

void HealerObject::changeATTACKState()
{
	nextState = std::make_unique<HealerATTACKState>(*this);
}

void HealerObject::changeSKILLState()
{
	nextState = std::make_unique<HealerSKILLState>(*this);
}
