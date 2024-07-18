#pragma once

#include "MonsterObject.h"

class MushroomObject : public MonsterObject
{
private:

public:
	MushroomObject(Room* parentRoom, int m_id);
	virtual ~MushroomObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

};

