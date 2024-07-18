#pragma once

#include "GameObjectBase.h"

class BossObject : public GameObjectBase
{
private:

public:
	BossObject(Room* parentRoom, int m_id);
	virtual ~BossObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

};

