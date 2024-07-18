#pragma once

#include "PlayerObject.h"

class HealerATTACKState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	HealerATTACKState(PlayerObject& player);
	virtual ~HealerATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class HealerSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	HealerSKILLState(PlayerObject& player);
	virtual ~HealerSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class HealerObject : public PlayerObject
{
private:

public:
	HealerObject(Room* parentRoom, int p_id);
	virtual ~HealerObject() = default;

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

