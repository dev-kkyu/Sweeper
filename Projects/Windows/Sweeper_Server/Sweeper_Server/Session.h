#pragma once

#include <memory>

#define ASIO_STANDALONE
#include "includes/asio.hpp"

#include "protocol.h"

class Session
	: public std::enable_shared_from_this<Session>		// �񵿱� �ݹ�(�ַ� �����Լ�) ȣ��ñ��� ��ü�� �����ֱ� ���Ͽ�(����Ʈ������) ��������� ��
{
private:
	asio::ip::tcp::socket socket;
	int my_id;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

public:
	Session(asio::ip::tcp::socket socket, int new_id);
	void start();
	void sendPacket(void* packet);

private:
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);
	void processPacket(unsigned char* packet);

};

