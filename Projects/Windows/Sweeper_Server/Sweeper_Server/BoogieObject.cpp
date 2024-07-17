#include "BoogieObject.h"

BoogieObject::BoogieObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::BOOGIE;
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
