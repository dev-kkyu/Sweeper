#pragma once

#include "PlayerObject.h"

#include <unordered_set>

class MageATTACKState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;

	std::unordered_set<GameObjectBase*> attackedObject;

public:
	MageATTACKState(PlayerObject& player);
	virtual ~MageATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class MageSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;

	std::unordered_set<GameObjectBase*> attackedObject;

public:
	MageSKILLState(PlayerObject& player);
	virtual ~MageSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class MageObject : public PlayerObject
{
private:

public:
	MageObject(Room* parentRoom, int p_id);
	virtual ~MageObject() = default;

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

