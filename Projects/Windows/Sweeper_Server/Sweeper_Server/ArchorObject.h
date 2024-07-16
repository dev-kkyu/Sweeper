#pragma once

#include "PlayerObject.h"

#include <unordered_map>

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
	friend class ArchorAttackState;
	friend class ArchorSKILLState;

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

	struct ArchorEffect {
		glm::vec3 pos;
		glm::vec3 dir;
		float accumTime = 0.f;
	};
	std::vector<ArchorEffect> archorEffects;

public:
	ArchorObject(Room* parentRoom, int p_id);
	virtual ~ArchorObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

private:
	virtual void changeATTACKState() override;
	virtual void changeSKILLState() override;

};

