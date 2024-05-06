#include "NetworkManager.h"

#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>

// tcp::socket 클래스의 전방선언이 불가하여, void*로 대체하는 대신 사용할 헬퍼함수
static asio::ip::tcp::socket& getSocket(void* p_asio_tcp_socket)
{
	return *static_cast<asio::ip::tcp::socket*>(p_asio_tcp_socket);
}

NetworkManager::NetworkManager()
{
	p_io_context = std::make_shared<asio::io_context>();
	p_socket = new asio::ip::tcp::socket{ *p_io_context };

	remain_size = 0;
	my_id = -1;
}

NetworkManager::~NetworkManager()
{
	delete (&getSocket(p_socket));
	p_io_context.reset();
}

NetworkManager& NetworkManager::getInstance()
{
	static NetworkManager instance;		// C++11 이후 Thread-Safe 하다.
	return instance;
}

void NetworkManager::connectServer(std::string ipAddress)
{
	asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(ipAddress), SERVER_PORT);
	asio::error_code ec;
	getSocket(p_socket).connect(endpoint, ec);		// connect는 비동기로 할 필요 없어보인다. 어차피 연결 안되면 바로 리턴됨.

	if (!ec) {		// connect 성공시
		std::cout << "Connect To : " << ipAddress << ":" << SERVER_PORT << " Succeeded." << std::endl;
		// Todo : ...
	}
	else {			// 실패시
		std::cerr << "Connect Error[" << ec << "]: " << ec.message() << std::endl;
		exit(-1);
	}
}

void NetworkManager::start(PLAYER_TYPE player_type)
{
	// 로그인 패킷 전송
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;
	p.player_type = player_type;
	sendPacket(&p);

	doRead();		// 수신하기를 시작한다.

}

void NetworkManager::poll()
{
	p_io_context->poll();
}

void NetworkManager::stop()
{
	p_io_context->stop();
	getSocket(p_socket).close();
}

void NetworkManager::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite의 람다 함수에서 해제해 줄 것이다.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

void NetworkManager::setPacketReceivedCallback(std::function<void(unsigned char*)> callback)
{
	processPacketFunc = callback;
}

void NetworkManager::doRead()
{
	getSocket(p_socket).async_read_some(asio::buffer(read_buffer),
		[this](asio::error_code ec, std::size_t length)
		{
			if (ec) {		// 실패했을 때
				if (ec.value() == asio::error::operation_aborted)
					return;
				std::cout << "Receive Error on Session[" << my_id << "]: [" << ec << "]: " << ec.message() << std::endl;
				std::cout << "네트워크 연결이 종료되었습니다" << std::endl;
				return;
			}

			int received = static_cast<int>(length);
			unsigned char* ptr = read_buffer;

			// 패킷 재조립
			while (received > 0) {
				int packet_size = ptr[0];
				if (remain_size > 0) {
					packet_size = remain_data[0];
				}

				if (remain_size + received >= packet_size) {							// 가지고 있는 데이터 크기가 처리할 데이터 크기 이상일때
					memcpy(remain_data + remain_size, ptr, packet_size - remain_size);	// 처리할 데이터만큼 패킷을 만들어준다.
					ptr += packet_size - remain_size;									// 다음에 처리할 데이터의 시작 위치로 미리 이동
					received -= packet_size - remain_size;								// 다음에 남은 데이터 크기를 저장
					processPacket(remain_data);											// 만든 패킷에 대한 처리하기
					remain_size = 0;													// remain_size는 received로 통합되었기 때문에 0으로 만들어준다.
				}
				else {
					memcpy(remain_data + remain_size, ptr, received);					// 다음에 처리할 데이터 저장해두기
					remain_size += received;
					received = 0;
				}
			}

			doRead();	// 다음 수신 데이터를 기다린다.
		});
}

void NetworkManager::doWrite(unsigned char* packet, std::size_t length)
{
	// 함수의 첫번째 인자로 들어가는 buffer는 비동기 작업 동안 메모리가 소멸되지 않도록 메모리를 프로그램이 관리해 줘야 한다.
	getSocket(p_socket).async_write_some(asio::buffer(packet, length),
		[this, packet, length](asio::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)		// send 완료 시
			{
				if (length != bytes_transferred) {
					std::cout << "Incomplete Send occured on session[" << my_id << "]. This session should be closed.\n";
				}
				delete[] packet;		// 작업이 완료되었으므로 더이상 필요없고, 놔두면 Leak이다.	// 소멸자가 없는 객체는 delete, delete[] 상관없음
			}
		});
}

void NetworkManager::processPacket(unsigned char* packet)
{
	// 패킷 처리 콜백 함수가 설정되어 있으면 호출
	if (processPacketFunc) {
		processPacketFunc(packet);
	}
}
