#pragma once

#include <memory>

#define ASIO_STANDALONE
#include "includes/asio.hpp"

#include "protocol.h"

class Room;
class PlayerObject;
class Session
	: public std::enable_shared_from_this<Session>		// �񵿱� �ݹ�(�ַ� �����Լ�) ȣ��ñ��� ��ü�� �����ֱ� ���Ͽ�(����Ʈ������) ��������� ��
{
private:
	asio::ip::tcp::socket socket;

	Room* parentRoom = nullptr;

	int player_id = -1;

	std::shared_ptr<PlayerObject> player;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

public:
	Session(asio::ip::tcp::socket socket);
	void start(Room* parentRoom, int player_id);
	void update(float elapsedTime);
	void sendPacket(void* packet);

private:
	void processPacket(unsigned char* packet);
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);

};

