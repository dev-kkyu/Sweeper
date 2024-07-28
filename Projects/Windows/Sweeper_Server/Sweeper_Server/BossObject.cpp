#include "BossObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

#include <iostream>

BossState::BossState(BossObject& boss)
	: boss{ boss }
{
}

void BossState::enter()
{
	SC_BOSS_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_BOSS_STATE;
	p.state = state;
	for (auto& a : boss.parentRoom->sessions) {			// 모든 플레이어에게 변경된 보스 State를 보내준다.
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session))
			session->sendPacket(&p);
	}
}

void BossState::update(float elapsedTime)
{
	if (state != BOSS_STATE::DIE) {
		if (boss.HP <= 0) {
			boss.changeDIEState();
		}
	}
}

void BossState::exit()
{
}

BOSS_STATE BossState::getState() const
{
	return state;
}

BossSLEEP::BossSLEEP(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::SLEEP;
}

void BossSLEEP::enter()
{
	BossState::enter();
}

void BossSLEEP::update(float elapsedTime)
{
	for (auto& a : boss.parentRoom->sessions) {
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidPlayer(session)) {
			auto myPos = boss.getPosition();
			auto playerPos = session->player->getPosition();

			float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
			float targetDist2 = glm::pow(8.f, 2.f);

			if (dist2 <= targetDist2) {		// 플레이어가 접근하면
				boss.changeWAKEUPState();
				break;
			}
		}
	}

	BossState::update(elapsedTime);
}

void BossSLEEP::exit()
{
	BossState::exit();
}

BossWAKEUP::BossWAKEUP(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::WAKEUP;

	stateAccumTime = 0.f;
}

void BossWAKEUP::enter()
{
	BossState::enter();
}

void BossWAKEUP::update(float elapsedTime)
{
	stateAccumTime += elapsedTime;

	if (stateAccumTime >= 2.825f) {
		boss.changeIDLEState();
	}

	BossState::update(elapsedTime);
}

void BossWAKEUP::exit()
{
	BossState::exit();
}

BossIDLE::BossIDLE(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::IDLE;
}

void BossIDLE::enter()
{
	BossState::enter();
}

void BossIDLE::update(float elapsedTime)
{
	for (int i = 0; i < 4; ++i) {
		std::shared_ptr<Session> session = boss.parentRoom->sessions[i].load();
		if (Room::isValidPlayer(session)) {
			BoundingBox boundingBox;
			boundingBox.setBound(1.f, 0.1f, 125.f, 107.f, 3.f, 21.f);	// 보스 방의 사각형
			// 보스 방에 있는 타겟만 검색한다
			if (boundingBox.isCollide(session->player->getBoundingBox())) {
				auto myPos = boss.getPosition();
				auto playerPos = session->player->getPosition();

				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				float targetDist2 = glm::pow(6.5f, 2.f);

				if (dist2 <= targetDist2) {		// 플레이어가 일정 거리 안이면
					boss.targetPlayer = i;
					boss.changeMOVEState();		// 타겟이 정해지고, MOVE 상태로 바뀐다
					break;
				}
			}
		}
	}

	BossState::update(elapsedTime);
}

void BossIDLE::exit()
{
	BossState::exit();
}

BossMOVE::BossMOVE(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::MOVE;
}

void BossMOVE::enter()
{
	BossState::enter();
}

void BossMOVE::update(float elapsedTime)
{
	if (boss.targetPlayer >= 0) {
		std::shared_ptr<Session> session1 = boss.parentRoom->sessions[boss.targetPlayer].load();
		if (Room::isValidPlayer(session1)) {
			BoundingBox boundingBox;
			boundingBox.setBound(1.f, 0.1f, 125.f, 107.f, 3.f, 21.f);	// 보스 방의 사각형
			// 플레이어가 보스 방의 사각형 내에 있을때만 따라간다
			if (boundingBox.isCollide(session1->player->getBoundingBox())) {
				auto myPos1 = boss.getPosition();
				auto playerPos1 = session1->player->getPosition();
				auto dir = playerPos1 - myPos1;
				auto dist = glm::length(dir);
				if (dist >= glm::epsilon<float>()) {			// 정상적인 거리일 때
					bool isMoveAble = true;
					if (dist > 5.f) {	// 거리가 조금 멀 때 새로운 타겟이 존재하면 타겟을 변경한다
						for (int i = 0; i < 4; ++i) {
							if (i == boss.targetPlayer)
								continue;
							std::shared_ptr<Session> session2 = boss.parentRoom->sessions[i].load();
							if (Room::isValidPlayer(session2)) {
								// 보스 방에 있는 타겟만 검색한다
								if (boundingBox.isCollide(session2->player->getBoundingBox())) {
									auto myPos2 = boss.getPosition();
									auto playerPos2 = session2->player->getPosition();

									float dist2 = glm::pow(myPos2.x - playerPos2.x, 2.f) + glm::pow(myPos2.z - playerPos2.z, 2.f);
									float targetDist2 = glm::pow(4.f, 2.f);
									if (dist2 <= targetDist2) {		// 새 타겟이 일정 거리 안이면
										boss.targetPlayer = i;		// 타겟을 변경해 준다
										isMoveAble = false;
										break;
									}
								}
							}
						}
					}
					else if (boss.isCollide(*session1->player)) {	// 충돌이면 상태 변경
						boss.changeAttackPattern(*session1->player);
						isMoveAble = false;
					}

					if (isMoveAble) {
						boss.setLook(dir);
						boss.moveForward(2.f * elapsedTime);		// 타겟을 향해 움직인다
					}
				}
			}
			else {
				boss.targetPlayer = -1;
				boss.changeIDLEState();
			}
		}
		else {
			boss.targetPlayer = -1;
			boss.changeIDLEState();
		}
	}
	else {
		boss.changeIDLEState();
	}

	BossState::update(elapsedTime);
}

void BossMOVE::exit()
{
	BossState::exit();
}

BossLEFTPUNCH::BossLEFTPUNCH(BossObject& boss)
	: BossState{ boss }, hitPlayer{}
{
	state = BOSS_STATE::LEFT_PUNCH;

	stateAccumTime = 0.f;
}

void BossLEFTPUNCH::enter()
{
	BossState::enter();
}

void BossLEFTPUNCH::update(float elapsedTime)
{
	stateAccumTime += elapsedTime;

	if (stateAccumTime >= 2.8f) {
		if (boss.targetPlayer >= 0) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[boss.targetPlayer].load();
			if (Room::isValidPlayer(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeAttackPattern(*session->player);
				}
				else {
					boss.changeMOVEState();
				}
			}
			else {
				boss.changeIDLEState();
			}
		}
		else {
			boss.changeIDLEState();
		}
	}
	else if (stateAccumTime >= 1.125f and stateAccumTime < 1.325f) {	// 충돌 판정
		auto bossPos = boss.getPosition();
		auto hitCenter = bossPos + boss.getLook() * 2.25f;	// 보스 앞쪽에 위치한다면
		for (int i = 0; i < 4; ++i) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[i].load();
			if (Room::isValidPlayer(session)) {			// 존재하는 플레이어에 대해서
				if (not hitPlayer[i]) {						// 아직 hit 한 적이 없다면
					auto playerPos = session->player->getPosition();
					playerPos.y = 0.f;
					float dist = glm::length(playerPos - hitCenter);
					if (dist <= session->player->getCollisionRadius() + 2.25f) {	// 공격 범위 내에 존재한다면
						auto destDir = bossPos - playerPos;
						if (glm::length(destDir) >= glm::epsilon<float>()) {
							hitPlayer[i] = true;				// 한번만 피격
							session->player->setLook(destDir);	// look을 변경
							session->player->onHit(boss, 100);	// Hit
						}
					}
				}
			}
		}
	}

	BossState::update(elapsedTime);
}

void BossLEFTPUNCH::exit()
{
	BossState::exit();
}

BossRIGHTPUNCH::BossRIGHTPUNCH(BossObject& boss)
	: BossState{ boss }, hitPlayer{}
{
	state = BOSS_STATE::RIGHT_PUNCH;

	stateAccumTime = 0.f;
}

void BossRIGHTPUNCH::enter()
{
	BossState::enter();
}

void BossRIGHTPUNCH::update(float elapsedTime)
{
	stateAccumTime += elapsedTime;

	if (stateAccumTime >= 3.4f) {
		if (boss.targetPlayer >= 0) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[boss.targetPlayer].load();
			if (Room::isValidPlayer(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeAttackPattern(*session->player);
				}
				else {
					boss.changeMOVEState();
				}
			}
			else {
				boss.changeIDLEState();
			}
		}
		else {
			boss.changeIDLEState();
		}
	}
	else if (stateAccumTime >= 1.355f and stateAccumTime < 1.555f) {	// 충돌 판정
		auto bossPos = boss.getPosition();
		auto hitCenter = bossPos + boss.getLook() * 2.25f;	// 보스 앞쪽에 위치한다면
		for (int i = 0; i < 4; ++i) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[i].load();
			if (Room::isValidPlayer(session)) {			// 존재하는 플레이어에 대해서
				if (not hitPlayer[i]) {						// 아직 hit 한 적이 없다면
					auto playerPos = session->player->getPosition();
					playerPos.y = 0.f;
					float dist = glm::length(playerPos - hitCenter);
					if (dist <= session->player->getCollisionRadius() + 2.25f) {	// 공격 범위 내에 존재한다면
						auto destDir = bossPos - playerPos;
						if (glm::length(destDir) >= glm::epsilon<float>()) {
							hitPlayer[i] = true;				// 한번만 피격
							session->player->setLook(destDir);	// look을 변경
							session->player->onHit(boss, 100);	// Hit
						}
					}
				}
			}
		}
	}

	BossState::update(elapsedTime);
}

void BossRIGHTPUNCH::exit()
{
	BossState::exit();
}

BossPUNCHDOWN::BossPUNCHDOWN(BossObject& boss)
	: BossState{ boss }, hitPlayer{}
{
	state = BOSS_STATE::PUNCH_DOWN;

	stateAccumTime = 0.f;
}

void BossPUNCHDOWN::enter()
{
	BossState::enter();
}

void BossPUNCHDOWN::update(float elapsedTime)
{
	stateAccumTime += elapsedTime;

	if (stateAccumTime >= 3.05f) {
		if (boss.targetPlayer >= 0) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[boss.targetPlayer].load();
			if (Room::isValidPlayer(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeAttackPattern(*session->player);
				}
				else {
					boss.changeMOVEState();
				}
			}
			else {
				boss.changeIDLEState();
			}
		}
		else {
			boss.changeIDLEState();
		}
	}
	else if (stateAccumTime >= 1.1f and stateAccumTime < 1.3f) {	// 충돌 판정
		auto bossPos = boss.getPosition();
		for (int i = 0; i < 4; ++i) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[i].load();
			if (Room::isValidPlayer(session)) {			// 존재하는 플레이어에 대해서
				if (not hitPlayer[i]) {						// 아직 hit 한 적이 없다면
					auto playerPos = session->player->getPosition();
					playerPos.y = 0.f;
					float dist = glm::length(playerPos - bossPos);				// 보스와 플레이어 사이의 거리가
					if (dist <= session->player->getCollisionRadius() + 4.5f) {	// 공격 범위 내에 존재한다면
						auto destDir = bossPos - playerPos;
						if (glm::length(destDir) >= glm::epsilon<float>()) {
							hitPlayer[i] = true;				// 한번만 피격
							session->player->setLook(destDir);	// look을 변경
							session->player->onHit(boss, 150);	// Hit
						}
					}
				}
			}
		}
	}

	BossState::update(elapsedTime);
}

void BossPUNCHDOWN::exit()
{
	BossState::exit();
}

BossDIE::BossDIE(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::DIE;
}

void BossDIE::enter()
{
	BossState::enter();

	SC_GAME_END_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GAME_END;
	p.is_win = true;
	for (auto& a : boss.parentRoom->sessions) {			// 모든 플레이어에게 게임 종료-승리 를 알려준다
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session))
			session->sendPacket(&p);
	}
	std::cout << "Room[" << boss.parentRoom->room_id << "] : 게임 승리 패킷 전송" << std::endl;

	boss.parentRoom->isEnd = true;
}

void BossDIE::update(float elapsedTime)
{
	BossState::update(elapsedTime);
}

void BossDIE::exit()
{
	BossState::exit();
}

BossObject::BossObject(Room* parentRoom)
	: GameObjectBase{ parentRoom, 0 }		// 보스는 하나이기 때문에, id가 필요 없다
{
	setPosition({ 12.25f, 0.f, 115.f });	// 클라와 동기화 해야 함
	setLook(glm::vec3{ 0.f, 0.f, -1.f });

	collisionRadius = 1.75f;

	maxHP = HP = MAX_HP_BOSS;

	currentState = std::make_unique<BossSLEEP>(*this);

	targetPlayer = -1;

	attackStateFlag = 0;

	isLastLeftPunch = false;
}

BossObject::~BossObject()
{
}

void BossObject::initialize()
{
}

bool BossObject::update(float elapsedTime)
{
	if (nextState) {
		currentState->exit();
		currentState = std::move(nextState);
		currentState->enter();
	}
	currentState->update(elapsedTime);

	if (dynamic_cast<BossMOVE*>(currentState.get())) {
		return true;
	}
	return false;
}

void BossObject::release()
{
}

void BossObject::onHit(const GameObjectBase& other, int damage)
{
	HP -= damage;
	std::cout << "보스가 [" << damage << "]의 피해를 입어 현재 체력 [" << HP << "]" << std::endl;

	SC_BOSS_HP_PACKET p;
	p.size = sizeof(p);
	p.type = SC_BOSS_HP;
	p.hp = HP;

	for (auto& a : parentRoom->sessions) {	// 모든 플레이어에게 boss의 hp를 보내준다
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session)) {
			session->sendPacket(&p);
		}
	}
}

BOSS_STATE BossObject::getBossState() const
{
	return currentState->getState();
}

void BossObject::changeSLEEPState()
{
	nextState = std::make_unique<BossSLEEP>(*this);
}

void BossObject::changeWAKEUPState()
{
	nextState = std::make_unique<BossWAKEUP>(*this);
}

void BossObject::changeIDLEState()
{
	nextState = std::make_unique<BossIDLE>(*this);
}

void BossObject::changeMOVEState()
{
	nextState = std::make_unique<BossMOVE>(*this);
}

void BossObject::changeLEFTPUNCHState()
{
	nextState = std::make_unique<BossLEFTPUNCH>(*this);
}

void BossObject::changeRIGHTPUNCHState()
{
	nextState = std::make_unique<BossRIGHTPUNCH>(*this);
}

void BossObject::changePUNCHDOWNState()
{
	nextState = std::make_unique<BossPUNCHDOWN>(*this);
}

void BossObject::changeDIEState()
{
	nextState = std::make_unique<BossDIE>(*this);
}

void BossObject::changeAttackPattern(const PlayerObject& target)
{
	{	// Attack을 할 때마다 보스가 보는 방향을 타겟에게로 바꿔준다
		auto targetPos = target.getPosition();
		targetPos.y = 0.f;
		auto myPos = getPosition();
		auto newDir = targetPos - myPos;
		if (glm::length(newDir) >= glm::epsilon<float>()) {
			setLook(newDir);

			SC_MOVE_BOSS_PACKET p;
			p.size = sizeof(p);
			p.type = SC_MOVE_BOSS;
			auto pos = myPos;
			auto look = getLook();
			p.pos_x = pos.x;
			p.pos_z = pos.z;
			p.dir_x = look.x;
			p.dir_z = look.z;
			for (auto& a : parentRoom->sessions) {		// 모든 세션에게 변화된 보스 위치 정보를 보내준다
				std::shared_ptr<Session> session = a.load();
				if (Room::isValidSession(session))
					session->sendPacket(&p);
			}
		}
	}

	switch (attackStateFlag)
	{
	case 0:
		changeLEFTPUNCHState();
		isLastLeftPunch = true;
		break;
	case 1:
		changeRIGHTPUNCHState();
		isLastLeftPunch = false;
		break;
	case 2:
		changePUNCHDOWNState();
		break;
	}

	if (HP <= MAX_HP_BOSS / 2) {
		if (2 == attackStateFlag) {
			if (isLastLeftPunch)
				attackStateFlag = 1;
			else
				attackStateFlag = 0;
		}
		else {
			attackStateFlag = 2;
		}
	}
	else {
		attackStateFlag = (attackStateFlag + 1) % 2;
	}

}
