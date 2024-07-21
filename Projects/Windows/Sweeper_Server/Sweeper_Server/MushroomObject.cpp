#include "MushroomObject.h"

MushroomObject::MushroomObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::MUSHROOM;

	hitDelayTime_ms = 1300;

	collisionRadius = 0.4f;

	maxHP = HP = MAX_HP_MONSTER_MUSHROOM;
}

MushroomObject::~MushroomObject()
{
}

void MushroomObject::initialize()
{
}

bool MushroomObject::update(float elapsedTime)
{
	return MonsterObject::update(elapsedTime);
}

void MushroomObject::release()
{
}

void MushroomObject::onHit(const GameObjectBase& other, int damage)
{
	MonsterObject::onHit(other, damage);
}
