#pragma once

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_LEFT		0x04
#define KEY_RIGHT		0x08
#define KEY_SPACE		0x10	// 사실 SPACE는 별도로 관리된다.
#define KEY_MOUSE_RIGHT	0x20

#include "GameObjectBase.h"

class Room;
class PlayerObject : public GameObjectBase
{
private:
	// 플레이어의 소속 정보, 플레이어는 세션이다
	Room* parentRoom;
	int my_id;

private:
	// 고정 상태인 경우 움직이지 않는다
	bool isFixedState = false;

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
	virtual ~PlayerObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;

	unsigned int getKeyState() const;
	bool getFixedState() const;
	void setFixedState(bool state);

	void processKeyInput(unsigned int key, bool is_pressed);
	void processMoveMouse(float move_x, float move_y);

};

