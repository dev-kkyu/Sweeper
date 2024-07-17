#pragma once

#include "GameObjectBase.h"

#include "protocol.h"

#include <chrono>

class MonsterObject : public GameObjectBase
{
protected:
	MONSTER_TYPE type;
	MONSTER_STATE state;
	std::chrono::steady_clock::time_point attackBeginTime;

	int targetPlayer;

	int hp;

public:
	MonsterObject(Room* parentRoom, int m_id);
	virtual ~MonsterObject();

	virtual void initialize() override = 0;
	virtual bool update(float elapsedTime) override = 0;
	virtual void release() override = 0;
	virtual void onHit(const GameObjectBase& other) override = 0;

	MONSTER_TYPE getMonsterType() const;

protected:
	void sendMonsterStatePacket();

};

