#include "BossObject.h"

BossObject::BossObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
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

void BossObject::onHit(const GameObjectBase& other)
{
}
