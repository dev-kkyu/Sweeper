#pragma once
#include <string>
#include <thread>
#include <memory>
#include <functional>

#define SERVER_PORT		7777
#define BUFF_SIZE		1024

namespace asio {
	class io_context;
}

class NetworkManager
{
private:
	std::shared_ptr<asio::io_context> p_io_context;
	void* p_socket;					// asio::ip::tcp::socket Ŭ������ �����̴�.

	int my_id;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

	// �񵿱� �۾��� ������
	std::thread worker_thread;

	// ��Ŷ ���Ž� ��Ŷ ó���� �ݹ� �Լ�
	std::function<void(unsigned char*)> processPacketFunc;

public:
	NetworkManager();
	~NetworkManager();

	// ����� �� �� ������ ���Ƶд�.
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

	void connectServer(std::string ipAddress);
	void start();
	void sendPacket(void* packet);
	void setPacketReceivedCallback(std::function<void(unsigned char*)> callback);
	void stop();

private:
	void runAsyncWork();
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);
	void processPacket(unsigned char* packet);

};

