#pragma once
#include <string>
#include <memory>
#include <functional>

#include "../../Sweeper_Server/Sweeper_Server/protocol.h"

namespace asio {
	class io_context;
}

// �̱��� Ŭ����
class NetworkManager
{
private:
	std::shared_ptr<asio::io_context> p_io_context;
	void* p_socket;					// asio::ip::tcp::socket Ŭ������ �����̴�.

	bool isStart;					// start ���Ŀ� poll�� �����Ѵ�.

	int my_id;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

	// ��Ŷ ���Ž� ��Ŷ ó���� �ݹ� �Լ�
	std::function<void(unsigned char*)> processPacketFunc;

private:			// �̱������� ����� ���� ������ �ܺ� ���� X
	NetworkManager();
	~NetworkManager();

	// ����� �� �� ������ ���Ƶд�.	// �̱����� ���ؼ��� �����ش�.
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

public:
	static NetworkManager& getInstance();	// �̱��� ��ü ��� ���� static �Լ�

public:
	void connectServer(std::string ipAddress);
	void start(PLAYER_TYPE player_type);
	void update();
	void stop();
	void sendPacket(void* packet);

	// read ���� ��, read�� �����͸� ó���� �Լ� ����
	void setPacketReceivedCallback(std::function<void(unsigned char*)> callback);

private:
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);
	void processPacket(unsigned char* packet);

};

