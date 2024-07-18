#pragma once

#include "MonsterObject.h"

class BoogieObject : public MonsterObject
{
private:

public:
	BoogieObject(Room* parentRoom, int m_id);
	virtual ~BoogieObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

};

