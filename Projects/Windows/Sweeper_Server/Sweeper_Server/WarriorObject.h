#pragma once

#include "PlayerObject.h"

class WarriorATTACKState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	WarriorATTACKState(PlayerObject& player);
	virtual ~WarriorATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class WarriorSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	WarriorSKILLState(PlayerObject& player);
	virtual ~WarriorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class WarriorObject : public PlayerObject
{
private:

public:
	WarriorObject(Room* parentRoom, int p_id);
	virtual ~WarriorObject() = default;

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

