#pragma once

#include "MonsterObject.h"

class BornDogObject : public MonsterObject
{
private:

public:
	BornDogObject(Room* parentRoom, int m_id);
	virtual ~BornDogObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

};

