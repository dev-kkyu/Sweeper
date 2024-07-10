#pragma once

#include "PlayerObject.h"

class MageAttackState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	MageAttackState(PlayerObject& player);
	virtual ~MageAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class MageSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
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
	virtual void onHit(const GameObjectBase& other) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

