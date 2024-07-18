#include "BossObject.h"

#include "Room.h"
#include "Session.h"

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
}

void BossWAKEUP::enter()
{
	BossState::enter();
}

void BossWAKEUP::update(float elapsedTime)
{
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
}

void BossLEFTPUNCH::enter()
{
	BossState::enter();
}

void BossLEFTPUNCH::update(float elapsedTime)
{
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
}

void BossRIGHTPUNCH::enter()
{
	BossState::enter();
}

void BossRIGHTPUNCH::update(float elapsedTime)
{
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

BossObject::BossObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
	hp = 100000;
}

BossObject::~BossObject()
{
}

void BossObject::initialize()
{
}

bool BossObject::update(float elapsedTime)
{
	return false;
}

void BossObject::release()
{
}

void BossObject::onHit(const GameObjectBase& other, int damage)
{
}
