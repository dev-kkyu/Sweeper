#pragma once

#include "GameObjectBase.h"

class MonsterObject : public GameObjectBase
{
private:
	int my_id;

public:
	MonsterObject(int m_id);
	virtual ~MonsterObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;

};

