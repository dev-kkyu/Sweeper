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

		// 몬스터와 충돌 검사
		for (auto& m : player.parentRoom->monsters) {
			if (0 != attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
				continue;
			auto monPos = m.second->getPosition();

			float dist = glm::sqrt(glm::pow(myPos.x - monPos.x, 2.f) + glm::pow(myPos.z - monPos.z, 2.f));
			float targetDist = 1.f + m.second->getCollisionRadius();		// 공격 범위 : 플레이어 앞쪽 1.f부터 반지름 1.f의 원
			if (dist <= targetDist) {	// 충돌
				m.second->onHit(player, player.attackDamage);
				attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
			}
		}
		// 보스와 충돌 검사
		if (0 == attackedObject.count(player.parentRoom->boss.get())) {	// 이전에 공격이 없었어야 한다
			auto monPos = player.parentRoom->boss->getPosition();

			float dist = glm::sqrt(glm::pow(myPos.x - monPos.x, 2.f) + glm::pow(myPos.z - monPos.z, 2.f));
			float targetDist = 1.f + player.parentRoom->boss->getCollisionRadius();		// 공격 범위 : 플레이어 앞쪽 1.f부터 반지름 1.f의 원
			if (dist <= targetDist) {	// 충돌
				player.parentRoom->boss->onHit(player, player.attackDamage);
				attackedObject.insert(player.parentRoom->boss.get());			// 한번만 공격이 들어가도록 한다
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
	maxHP = HP = MAX_HP_PLAYER_HEALER;

	attackDamage = 100;
	skillDamage = 30;		// 플레이어에게 주는 힐량
}

void HealerObject::initialize()
{
}

bool HealerObject::update(float elapsedTime)
{
	// 이펙트 시간 업데이트
	for (auto& hEffect : healerEffects) {
		hEffect.accumTime += elapsedTime;
	}
	// 이펙트 수명 관리
	std::list<std::list<HealerObject::HealerEffect>::iterator> deleteEffects;
	for (auto itr = healerEffects.begin(); itr != healerEffects.end(); ++itr) {
		if (itr->accumTime >= 2.35f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		healerEffects.erase(itr);
	}
	// 이펙트 유지되는동안 힐
	for (auto& hEffect : healerEffects) {
		if (hEffect.accumTime >= 0.f) {
			for (auto& a : parentRoom->sessions) {
				std::shared_ptr<Session> session = a.load();
				if (Room::isValidSession(session)) {
					auto playerPos = session->player->getPosition();
					float dist = glm::length(playerPos - hEffect.pos);
					if (dist <= session->player->getCollisionRadius() + 2.f) {
						// 이펙트로부터 반지름 2.f 안에 있다면 힐
						PlayerObject* ptrPlayer = session->player.get();
						auto nowTime = std::chrono::steady_clock::now();
						if (hEffect.healedPlayer.find(ptrPlayer) == hEffect.healedPlayer.end()) {
							hEffect.healedPlayer.try_emplace(ptrPlayer, nowTime);
							short nowHP = session->player->getHP();
							session->player->setHP(nowHP + skillDamage);
							session->player->broadcastMyHP();
						}	// 0.75초마다 힐
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
