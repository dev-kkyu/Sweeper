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
	for (auto& a : boss.parentRoom->sessions) {			// ��� �÷��̾�� ����� �÷��̾� State�� �����ش�.
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

			if (dist2 <= targetDist2) {		// �÷��̾ �����ϸ�
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
			BoundingBox boundingBox;
			boundingBox.setBound(1.f, 0.1f, 125.f, 107.f, 3.f, 21.f);	// ���� ���� �簢��
			// ���� �濡 �ִ� Ÿ�ٸ� �˻��Ѵ�
			if (boundingBox.isCollide(session->player->getBoundingBox())) {
				auto myPos = boss.getPosition();
				auto playerPos = session->player->getPosition();

				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				float targetDist2 = glm::pow(6.5f, 2.f);

				if (dist2 <= targetDist2) {		// �÷��̾ ���� �Ÿ� ���̸�
					boss.targetPlayer = i;
					boss.changeMOVEState();		// Ÿ���� ��������, MOVE ���·� �ٲ��
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
		if (Room::isValidSession(session1)) {
			BoundingBox boundingBox;
			boundingBox.setBound(1.f, 0.1f, 125.f, 107.f, 3.f, 21.f);	// ���� ���� �簢��
			// �÷��̾ ���� ���� �簢�� ���� �������� ���󰣴�
			if (boundingBox.isCollide(session1->player->getBoundingBox())) {
				auto myPos1 = boss.getPosition();
				auto playerPos1 = session1->player->getPosition();
				auto dir = playerPos1 - myPos1;
				auto dist = glm::length(dir);
				if (dist >= glm::epsilon<float>()) {			// �������� �Ÿ��� ��
					bool isMoveAble = true;
					if (dist > 5.f) {	// �Ÿ��� ���� �� �� ���ο� Ÿ���� �����ϸ� Ÿ���� �����Ѵ�
						for (int i = 0; i < 4; ++i) {
							if (i == boss.targetPlayer)
								continue;
							std::shared_ptr<Session> session2 = boss.parentRoom->sessions[i].load();
							if (Room::isValidSession(session2)) {
								// ���� �濡 �ִ� Ÿ�ٸ� �˻��Ѵ�
								if (boundingBox.isCollide(session2->player->getBoundingBox())) {
									auto myPos2 = boss.getPosition();
									auto playerPos2 = session2->player->getPosition();

									float dist2 = glm::pow(myPos2.x - playerPos2.x, 2.f) + glm::pow(myPos2.z - playerPos2.z, 2.f);
									float targetDist2 = glm::pow(4.f, 2.f);
									if (dist2 <= targetDist2) {		// �� Ÿ���� ���� �Ÿ� ���̸�
										boss.targetPlayer = i;		// Ÿ���� ������ �ش�
										isMoveAble = false;
										break;
									}
								}
							}
						}
					}
					else if (boss.isCollide(*session1->player)) {	// �浹�̸� ���� ����
						boss.changeAttackPattern();
						isMoveAble = false;
					}

					if (isMoveAble) {
						boss.setLook(dir);
						boss.moveForward(2.f * elapsedTime);		// Ÿ���� ���� �����δ�
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
					boss.changeAttackPattern();
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

	if (stateAccumTime >= 3.4f) {
		if (boss.targetPlayer >= 0) {
			std::shared_ptr<Session> session = boss.parentRoom->sessions[boss.targetPlayer].load();
			if (Room::isValidSession(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeAttackPattern();
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
			if (Room::isValidSession(session)) {
				if (boss.isCollide(*session->player)) {
					boss.changeAttackPattern();
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
	: GameObjectBase{ parentRoom, 0 }		// ������ �ϳ��̱� ������, id�� �ʿ� ����
{
	setPosition({ 12.25f, 0.f, 115.f });	// Ŭ��� ����ȭ �ؾ� ��
	setLook(glm::vec3{ 0.f, 0.f, -1.f });

	collisionRadius = 1.75f;

	maxHP = HP = MAX_HP_BOSS;

	currentState = std::make_unique<BossSLEEP>(*this);

	targetPlayer = -1;

	attackStateFlag = 0;
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
	std::cout << "������ [" << damage << "]�� ���ظ� �Ծ� ���� ü�� [" << HP << "]" << std::endl;

	SC_BOSS_HP_PACKET p;
	p.size = sizeof(p);
	p.type = SC_BOSS_HP;
	p.hp = HP;

	for (auto& a : parentRoom->sessions) {	// ��� �÷��̾�� boss�� hp�� �����ش�
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

void BossObject::changeAttackPattern()
{
	switch (attackStateFlag)
	{
	case 0:
		changeLEFTPUNCHState();
		break;
	case 1:
		changeRIGHTPUNCHState();
		break;
	case 2:
		changePUNCHDOWNState();
		break;
	}

	if (HP <= 5000) {
		attackStateFlag = (attackStateFlag + 1) % 3;
	}
	else {
		attackStateFlag = (attackStateFlag + 1) % 2;
	}

}
