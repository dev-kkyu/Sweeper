#pragma once

#include "PlayerObject.h"

#include <unordered_set>
#include <unordered_map>

class HealerATTACKState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;

	std::unordered_set<GameObjectBase*> attackedObject;

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
	friend class HealerSKILLState;

private:
	struct HealerEffect {
		glm::vec3 pos;
		float accumTime = 0.f;
		std::unordered_map<PlayerObject*, std::chrono::steady_clock::time_point> healedPlayer;
	};

	std::list<HealerEffect> healerEffects;

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

