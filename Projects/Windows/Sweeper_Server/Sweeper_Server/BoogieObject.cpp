#include "BoogieObject.h"

BoogieObject::BoogieObject(Room* parentRoom, int m_id)
	: MonsterObject{ parentRoom, m_id }
{
	type = MONSTER_TYPE::BOOGIE;

	hitDelayTime_ms = 1100;

	collisionRadius = 0.4f;

	maxHP = HP = MAX_HP_MONSTER_BOOGIE;

	attackDamage = 50;
	attackDelayTime_ms = 1150;
	attackPointTime_ms = 310;
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

void BoogieObject::onHit(const GameObjectBase& other, int damage)
{
	MonsterObject::onHit(other, damage);
}
