#include "GoblinObject.h"

GoblinObject::GoblinObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::GOBLIN;
}

GoblinObject::~GoblinObject()
{
}

void GoblinObject::initialize()
{
}

bool GoblinObject::update(float elapsedTime)
{
	return MonsterObject::update(elapsedTime);
}

void GoblinObject::release()
{
}

void GoblinObject::onHit(const GameObjectBase& other)
{
	MonsterObject::onHit(other);
}
