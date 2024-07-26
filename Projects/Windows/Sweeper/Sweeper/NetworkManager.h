#pragma once
#include <string>
#include <memory>
#include <functional>

#include "../../Sweeper_Server/Sweeper_Server/protocol.h"

namespace asio {
	class io_context;
}

// 싱글톤 클래스
class NetworkManager
{
private:
	std::shared_ptr<asio::io_context> p_io_context;
	void* p_socket;					// asio::ip::tcp::socket 클래스가 원형이다.

	bool isStart;					// start 이후에 poll을 시작한다.

	int my_id;

	// recv용 변수
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

	// 패킷 수신시 패킷 처리할 콜백 함수
	std::function<void(unsigned char*)> processPacketFunc;

private:			// 싱글톤으로 만들기 위해 생성자 외부 노출 X
	NetworkManager();
	~NetworkManager();

	// 복사는 할 일 없으니 막아둔다.	// 싱글톤을 위해서도 막아준다.
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

public:
	static NetworkManager& getInstance();	// 싱글톤 객체 얻기 위한 static 함수

public:
	void connectServer(std::string ipAddress);
	void start(PLAYER_TYPE player_type);
	void update();
	void stop();
	void sendPacket(void* packet);

	// read 성공 시, read한 데이터를 처리할 함수 지정
	void setPacketReceivedCallback(std::function<void(unsigned char*)> callback);

private:
	void doRead();
	void doWrite(unsigned char* packet, std::size_t length);
	void processPacket(unsigned char* packet);

};

