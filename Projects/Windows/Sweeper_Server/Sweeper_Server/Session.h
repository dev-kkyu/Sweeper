#pragma once

#include <memory>

#define ASIO_STANDALONE
#include "includes/asio.hpp"

#include "protocol.h"

class Room;
class PlayerObject;
class Session
	: public std::enable_shared_from_this<Session>		// 비동기 콜백(주로 람다함수) 호출시까지 객체가 남아있기 위하여(스마트포인터) 설정해줘야 함
{
public:
	PLAYER_TYPE player_type;	// Todo: 추후 플레이어를 상속받는 각 플레이어 클래스로 아래 player를 생성할 것이다
	std::shared_ptr<PlayerObject> player;	// 플레이어가 생성되었는지 여부에 따라 로그인 패킷이 날라왔는지 확인한다.

private:
	asio::ip::tcp::socket socket;

	Room* parentRoom = nullptr;

	int player_id = -1;

	bool isRun = false;

	// recv용 변수
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

public:
	Session(asio::ip::tcp::socket socket);
	void start(Room* parentRoom, int player_id);
	void update(float elapsedTime);
	void sendPacket(void* packet);

private:
	bool processPacket(unsigned char* packet);
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);

};

