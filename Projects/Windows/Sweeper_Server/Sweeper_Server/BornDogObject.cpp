#include "BornDogObject.h"

BornDogObject::BornDogObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::BORNDOG;

	hitDelayTime_ms = 1200;

	collisionRadius = 1.375f;

	maxHP = HP = MAX_HP_MONSTER_BORNDOG;
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

void BornDogObject::onHit(const GameObjectBase& other, int damage)
{
	MonsterObject::onHit(other, damage);
}
