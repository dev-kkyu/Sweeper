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
public:
	PLAYER_TYPE player_type;	// Todo: ���� �÷��̾ ��ӹ޴� �� �÷��̾� Ŭ������ �Ʒ� player�� ������ ���̴�
	std::shared_ptr<PlayerObject> player;	// �÷��̾ �����Ǿ����� ���ο� ���� �α��� ��Ŷ�� ����Դ��� Ȯ���Ѵ�.

private:
	asio::ip::tcp::socket socket;

	Room* parentRoom = nullptr;

	int player_id = -1;

	bool isRun = false;

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
	bool processPacket(unsigned char* packet);
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);

};

