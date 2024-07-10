#pragma once

#include "PlayerObject.h"

class ArchorAttackState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	ArchorAttackState(PlayerObject& player);
	virtual ~ArchorAttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class ArchorSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	ArchorSKILLState(PlayerObject& player);
	virtual ~ArchorSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class ArchorObject : public PlayerObject
{
private:

public:
	ArchorObject(Room* parentRoom, int p_id);
	virtual ~ArchorObject() = default;

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

