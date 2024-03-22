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

	unsigned int keyState = 0;

	float startXpos = 0.f;
	float startYpos = 0.f;

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime) override;
	virtual void release() override;

	void processKeyInput(unsigned int key, bool is_pressed);
	//void setStartMousePos(float xpos, float ypos);			// 일단 안써서 빼뒀다
	//void processMouseCursor(float xpos, float ypos);

};

