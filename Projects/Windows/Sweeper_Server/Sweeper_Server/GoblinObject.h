#pragma once

#include "MonsterObject.h"

class GoblinObject : public MonsterObject
{
private:

public:
	GoblinObject(Room* parentRoom, int m_id);
	virtual ~GoblinObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

};

