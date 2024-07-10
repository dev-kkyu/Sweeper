#pragma once

#include "GameObjectBase.h"
#include <chrono>

#include "protocol.h"	// PLAYER_STATE

class PlayerObject;
class StateMachine
{
protected:
	PLAYER_STATE state;
	PlayerObject& player;

public:
	StateMachine(PlayerObject& player);
	virtual ~StateMachine() = default;

	virtual void enter() = 0;
	virtual void update(float elapsedTime) = 0;
	virtual void exit() = 0;
	virtual PLAYER_STATE getState() const final;
};

class IDLEState : public StateMachine
{
public:
	IDLEState(PlayerObject& player);
	virtual ~IDLEState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class RUNState : public StateMachine
{
private:
	// 움직이는 속도	// 값은 생성자에서
	float maxMoveSpeed;		// 최대 속도
	float moveSpeed;		// 현재 속도
	float acceleration;		// 현재 가속도
	glm::vec3 lastDirection;	// 움직이던 방향 (키를 뗐을 때 갈 방향, xz만 사용)

public:
	RUNState(PlayerObject& player);
	virtual ~RUNState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;

};

class DASHState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;

	int accFlag;			// 가속 플래그 (증가, 감소)

	// 움직이는 속도	// 값은 생성자에서
	float maxMoveSpeed;		// 최대 속도
	float moveSpeed;		// 현재 속도
	float acceleration;		// 현재 가속도
	glm::vec3 direction;	// 이동할 방향 (xz만 사용)

public:
	DASHState(PlayerObject& player);
	virtual ~DASHState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class PlayerObject : public GameObjectBase
{
	friend class StateMachine;
	friend class IDLEState;
	friend class RUNState;
	friend class DASHState;
	friend class WarriorAttackState;
	friend class WarriorSKILLState;
	friend class ArchorAttackState;
	friend class ArchorSKILLState;
	friend class MageAttackState;
	friend class MageSKILLState;
	friend class HealerAttackState;
	friend class HealerSKILLState;

protected:
	// 상태 관리
	std::unique_ptr<StateMachine> currentState;
	std::unique_ptr<StateMachine> nextState;

	// 키가 눌려진 상태를 종합한다.
	unsigned int keyState = 0;

	// 점프 관련 변수
	bool runJump = false;
	// 값은 생성자에서 넣어준다.
	float gravity;			// 중력 가속도 (m/s^2)
	float jumpSpeed;		// 점프 시작 속도 (m/s)	-> 루트(2 * g * h) -> h == 최대높이
	float velocity;			// 현재 수직 속도

public:
	PlayerObject(Room* parentRoom, int p_id);
	virtual ~PlayerObject() = default;

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

	unsigned int getKeyState() const;

	void processKeyInput(unsigned int key, bool is_pressed);

protected:
	void rotateAndMoveToDirection(const glm::vec3& direction, float moveSpeed, float elapsedTime);
	void moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime);

	virtual void changeIDLEState() final;
	virtual void changeRUNState() final;
	virtual void changeDASHState() final;
	virtual void changeATTACKState() = 0;
	virtual void changeSKILLState() = 0;

};
