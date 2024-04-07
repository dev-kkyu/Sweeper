#pragma once

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_LEFT		0x04
#define KEY_RIGHT		0x08
#define KEY_SPACE		0x10	// ��� SPACE�� ������ �����ȴ�.
#define KEY_MOUSE_RIGHT	0x20

#include "GameObjectBase.h"

class PlayerObject : public GameObjectBase
{
private:
	// ���� ������ ��� �������� �ʴ´�
	bool isFixedState = false;

	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

	// ���� ���� ����
	bool runJump = false;
	// ���� �����ڿ��� �־��ش�.
	float gravity;			// �߷� ���ӵ� (m/s^2)
	float jumpSpeed;		// ���� ���� �ӵ� (m/s)	-> ��Ʈ(2 * g * h) -> h == �ִ����
	float velocity;			// ���� ���� �ӵ�

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime) override;
	virtual void release() override;

	unsigned int getKeyState() const;
	bool getFixedState() const;
	void setFixedState(bool state);

	void processKeyInput(unsigned int key, bool is_pressed);
	void processMoveMouse(float move_x, float move_y);

};

