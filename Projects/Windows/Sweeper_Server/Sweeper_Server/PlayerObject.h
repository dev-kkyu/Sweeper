#pragma once

#define KEY_UP		0x01
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x08
#define KEY_SPACE	0x10	// 사실 SPACE는 별도로 관리된다.

#include "GameObjectBase.h"

class PlayerObject : public GameObjectBase
{
private:

	// 키가 눌려진 상태를 종합한다.
	unsigned int keyState = 0;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime) override;
	virtual void release() override;

	void processKeyInput(unsigned int key, bool is_pressed);
	void processMoveMouse(float move_x, float move_y);

};

