#pragma once

#include "GameObjectBase.h"

#include "protocol.h"

#include <chrono>

class MonsterObject : public GameObjectBase
{
private:
	MONSTER_TYPE type;
	MONSTER_STATE state;
	std::chrono::steady_clock::time_point attackBeginTime;

	int hp;

public:
	MonsterObject(Room* parentRoom, int m_id, MONSTER_TYPE type);
	virtual ~MonsterObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

	MONSTER_TYPE getMonsterType() const;

private:
	void sendMonsterStatePacket();

};

