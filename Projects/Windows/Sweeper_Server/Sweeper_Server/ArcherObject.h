#pragma once

#include "PlayerObject.h"

#include <unordered_map>
#include <unordered_set>

class ArcherATTACKState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	ArcherATTACKState(PlayerObject& player);
	virtual ~ArcherATTACKState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class ArcherSKILLState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	ArcherSKILLState(PlayerObject& player);
	virtual ~ArcherSKILLState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class ArcherObject : public PlayerObject
{
	friend class ArcherATTACKState;
	friend class ArcherSKILLState;

private:
	static int arrowID;

	struct Arrow {
		std::chrono::steady_clock::time_point spawnTime;
		glm::vec3 pos;
		glm::vec3 dir;
		Arrow();
		Arrow(glm::vec3 pos, glm::vec3 dir);
	};
	std::unordered_map<int, Arrow> arrowObjects;

	struct ArcherEffect {
		glm::vec3 pos;
		glm::vec3 dir;
		float accumTime = 0.f;
		std::unordered_set<GameObjectBase*> attackedObject;
	};
	std::vector<ArcherEffect> archerEffects;

public:
	ArcherObject(Room* parentRoom, int p_id);
	virtual ~ArcherObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

