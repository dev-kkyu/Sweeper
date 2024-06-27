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
	virtual void exit() const final;
	virtual PLAYER_STATE getState() const final;
};

class IDLEState : public StateMachine
{
public:
	IDLEState(PlayerObject& player);
	virtual ~IDLEState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
};

class RUNState : public StateMachine
{
public:
	RUNState(PlayerObject& player);
	virtual ~RUNState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
};

class DASHState : public StateMachine
{
public:
	DASHState(PlayerObject& player);
	virtual ~DASHState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
};

class AttackState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point attackBeginTime;
public:
	AttackState(PlayerObject& player);
	virtual ~AttackState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
};

class PlayerObject : public GameObjectBase
{
	friend class StateMachine;
	friend class IDLEState;
	friend class RUNState;
	friend class DASHState;
	friend class AttackState;

private:
	// 상태 관리
	std::unique_ptr<StateMachine> currentState;
	std::unique_ptr<StateMachine> nextState;

	// 키가 눌려진 상태를 종합한다.
	unsigned int keyState = 0;

	// 움직이는 속도
	float moveSpeed = 0.f;

	// 점프 관련 변수
	bool runJump = false;
	// 값은 생성자에서 넣어준다.
	float gravity;			// 중력 가속도 (m/s^2)
	float jumpSpeed;		// 점프 시작 속도 (m/s)	-> 루트(2 * g * h) -> h == 최대높이
	float velocity;			// 현재 수직 속도

public:
	PlayerObject(Room* parentRoom, int p_id);
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

	unsigned int getKeyState() const;

	void processKeyInput(unsigned int key, bool is_pressed);

private:
	void rotateAndMoveToDirection(const glm::vec3& direction, float elapsedTime);

};
