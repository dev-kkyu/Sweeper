#include "MushroomObject.h"

MushroomObject::MushroomObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::MUSHROOM;

	collisionRadius = 0.4f;
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

void MushroomObject::onHit(const GameObjectBase& other)
{
	MonsterObject::onHit(other);
}
