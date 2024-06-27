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
	// �����̴� �ӵ�	// ���� �����ڿ���
	float maxMoveSpeed;		// �ִ� �ӵ�
	float moveSpeed;		// ���� �ӵ�
	float acceleration;		// ���� ���ӵ�
	glm::vec3 lastDirection;	// �����̴� ���� (Ű�� ���� �� �� ����, xz�� ���)

public:
	RUNState(PlayerObject& player);
	virtual ~RUNState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;

private:
	void rotateAndMoveToDirection(const glm::vec3& direction, float elapsedTime);

};

class DASHState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	DASHState(PlayerObject& player);
	virtual ~DASHState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class AttackState : public StateMachine
{
private:
	std::chrono::steady_clock::time_point stateBeginTime;
public:
	AttackState(PlayerObject& player);
	virtual ~AttackState() = default;

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
	friend class AttackState;

private:
	// ���� ����
	std::unique_ptr<StateMachine> currentState;
	std::unique_ptr<StateMachine> nextState;

	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

	// ���� ���� ����
	bool runJump = false;
	// ���� �����ڿ��� �־��ش�.
	float gravity;			// �߷� ���ӵ� (m/s^2)
	float jumpSpeed;		// ���� ���� �ӵ� (m/s)	-> ��Ʈ(2 * g * h) -> h == �ִ����
	float velocity;			// ���� ���� �ӵ�

public:
	PlayerObject(Room* parentRoom, int p_id);
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

	unsigned int getKeyState() const;

	void processKeyInput(unsigned int key, bool is_pressed);

};
