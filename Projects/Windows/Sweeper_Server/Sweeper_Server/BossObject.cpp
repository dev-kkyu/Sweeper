#include "BossObject.h"

BossState::BossState(BossObject& boss)
	: boss{ boss }
{
}

BOSS_STATE BossState::getState() const
{
	return state;
}

BossSLEEP::BossSLEEP(BossObject& boss)
	: BossState{ boss }
{
}

void BossSLEEP::enter()
{
}

void BossSLEEP::update(float elapsedTime)
{
}

void BossSLEEP::exit()
{
}

BossWAKEUP::BossWAKEUP(BossObject& boss)
	: BossState{ boss }
{
}

void BossWAKEUP::enter()
{
}

void BossWAKEUP::update(float elapsedTime)
{
}

void BossWAKEUP::exit()
{
}

BossIDLE::BossIDLE(BossObject& boss)
	: BossState{ boss }
{
}

void BossIDLE::enter()
{
}

void BossIDLE::update(float elapsedTime)
{
}

void BossIDLE::exit()
{
}

BossMOVE::BossMOVE(BossObject& boss)
	: BossState{ boss }
{
}

void BossMOVE::enter()
{
}

void BossMOVE::update(float elapsedTime)
{
}

void BossMOVE::exit()
{
}

BossLEFTPUNCH::BossLEFTPUNCH(BossObject& boss)
	: BossState{ boss }
{
}

void BossLEFTPUNCH::enter()
{
}

void BossLEFTPUNCH::update(float elapsedTime)
{
}

void BossLEFTPUNCH::exit()
{
}

BossRIGHTPUNCH::BossRIGHTPUNCH(BossObject& boss)
	: BossState{ boss }
{
}

void BossRIGHTPUNCH::enter()
{
}

void BossRIGHTPUNCH::update(float elapsedTime)
{
}

void BossRIGHTPUNCH::exit()
{
}

BossPUNCHDOWN::BossPUNCHDOWN(BossObject& boss)
	: BossState{ boss }
{
}

void BossPUNCHDOWN::enter()
{
}

void BossPUNCHDOWN::update(float elapsedTime)
{
}

void BossPUNCHDOWN::exit()
{
}

BossDIE::BossDIE(BossObject& boss)
	: BossState{ boss }
{
}

void BossDIE::enter()
{
}

void BossDIE::update(float elapsedTime)
{
}

void BossDIE::exit()
{
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
