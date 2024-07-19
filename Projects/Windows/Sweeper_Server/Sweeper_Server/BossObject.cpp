#include "BossObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

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
	for (auto& a : boss.parentRoom->sessions) {			// 모든 플레이어에게 변경된 플레이어 State를 보내준다.
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session))
			session->sendPacket(&p);
	}
}

void BossState::update(float elapsedTime)
{
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
		if (Room::isValidSession(session)) {
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
		if (Room::isValidSession(session)) {
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
		if (Room::isValidSession(session1)) {
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
						if (Room::isValidSession(session2)) {
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
				else if (boss.isCollide(*session1->player)) {	// 충돌이면 상태 변경
					boss.changeLEFTorRIGHTState();
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
		boss.changeIDLEState();
	}

	BossState::update(elapsedTime);
}

void BossMOVE::exit()
{
	BossState::exit();
}

BossLEFTPUNCH::BossLEFTPUNCH(BossObject& boss)
	: BossState{ boss }
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
			if (Room::isValidSession(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeLEFTorRIGHTState();
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

	BossState::update(elapsedTime);
}

void BossLEFTPUNCH::exit()
{
	BossState::exit();
}

BossRIGHTPUNCH::BossRIGHTPUNCH(BossObject& boss)
	: BossState{ boss }
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

	if (stateAccumTime >= 2.8f) {
		if (boss.targetPlayer >= 0) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[boss.targetPlayer].load();
			if (Room::isValidSession(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeLEFTorRIGHTState();
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

	BossState::update(elapsedTime);
}

void BossRIGHTPUNCH::exit()
{
	BossState::exit();
}

BossPUNCHDOWN::BossPUNCHDOWN(BossObject& boss)
	: BossState{ boss }
{
	state = BOSS_STATE::PUNCH_DOWN;
}

void BossPUNCHDOWN::enter()
{
	BossState::enter();
}

void BossPUNCHDOWN::update(float elapsedTime)
{
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
	hp = 100000;

	currentState = std::make_unique<BossSLEEP>(*this);

	targetPlayer = -1;

	stateFlag = false;
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

void BossObject::changeLEFTorRIGHTState()
{
	if (stateFlag)
		changeRIGHTPUNCHState();
	else
		changeLEFTPUNCHState();

	stateFlag = not stateFlag;
}
