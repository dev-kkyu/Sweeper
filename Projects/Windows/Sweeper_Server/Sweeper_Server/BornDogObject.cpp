#include "BornDogObject.h"

BornDogObject::BornDogObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::BORNDOG;
}

BornDogObject::~BornDogObject()
{
}

void BornDogObject::initialize()
{
}

bool BornDogObject::update(float elapsedTime)
{
	return MonsterObject::update(elapsedTime);
}

void BornDogObject::release()
{
}

void BornDogObject::onHit(const GameObjectBase& other)
{
	MonsterObject::onHit(other);
}
