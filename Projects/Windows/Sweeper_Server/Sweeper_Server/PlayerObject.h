#pragma once

#include "GameObjectBase.h"
#include <chrono>

class PlayerObject : public GameObjectBase
{
private:
	// ���� ������ ��� �������� �ʴ´�
	bool isFixedState = false;

	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

	// �����̴� �ӵ�
	float moveSpeed = 0.f;

	// ���� ���� ����
	bool runJump = false;
	// ���� �����ڿ��� �־��ش�.
	float gravity;			// �߷� ���ӵ� (m/s^2)
	float jumpSpeed;		// ���� ���� �ӵ� (m/s)	-> ��Ʈ(2 * g * h) -> h == �ִ����
	float velocity;			// ���� ���� �ӵ�

	// ���� ���� ����
	bool isAttack = false;
	std::chrono::steady_clock::time_point attackBeginTime;

public:
	PlayerObject(Room* parentRoom, int p_id);
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other) override;

	unsigned int getKeyState() const;
	bool getFixedState() const;
	void setFixedState(bool state);
	void setAttackStart();

	void processKeyInput(unsigned int key, bool is_pressed);

private:
	void rotateAndMoveToDirection(const glm::vec3& direction, float elapsedTime);

};
