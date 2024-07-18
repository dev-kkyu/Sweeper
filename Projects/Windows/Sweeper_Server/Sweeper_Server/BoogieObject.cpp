#include "BoogieObject.h"

BoogieObject::BoogieObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::BOOGIE;

	hitDelayTime_ms = 1100;

	collisionRadius = 0.4f;
}

BoogieObject::~BoogieObject()
{
}

void BoogieObject::initialize()
{
}

bool BoogieObject::update(float elapsedTime)
{
	return MonsterObject::update(elapsedTime);
}

void BoogieObject::release()
{
}

void BoogieObject::onHit(const GameObjectBase& other)
{
	MonsterObject::onHit(other);
}
