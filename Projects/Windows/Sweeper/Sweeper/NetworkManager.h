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
	void* p_socket;					// asio::ip::tcp::socket 클래스가 원형이다.

	int my_id;

	// recv용 변수
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

	// 비동기 작업용 스레드
	std::thread worker_thread;

	// 패킷 수신시 패킷 처리할 콜백 함수
	std::function<void(unsigned char*)> processPacketFunc;

public:
	NetworkManager();
	~NetworkManager();

	// 복사는 할 일 없으니 막아둔다.
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

