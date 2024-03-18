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
	void* p_steady_timer;

	int my_id;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

	std::function<void()> drawLoopFunc;							// ��ο� ���� �Լ�
	std::function<void(unsigned char*)> processPacketFunc;		// ��Ŷ ���Ž� ��Ŷ ó���� �ݹ� �Լ�


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
	void start();
	void run();
	void sendPacket(void* packet);
	void stop();

	// read ���� ��, read�� �����͸� ó���� �Լ� ����
	void setPacketReceivedCallback(std::function<void(unsigned char*)> callback);
	// Ŭ���̾�Ʈ ��ο� �Լ� ����
	void setDrawLoopFunc(std::function<void()> func);

private:
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);
	void drawLoop();
	void processPacket(unsigned char* packet);

};

