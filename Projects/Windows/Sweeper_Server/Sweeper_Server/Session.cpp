#include "Session.h"

#include <iostream>
#include <array>

// 외부 전역 변수
extern std::array<std::shared_ptr<Session>, MAX_USER> players;		// Server.cpp

Session::Session(asio::ip::tcp::socket socket, int new_id) : socket{ std::move(socket) }, my_id{ new_id }
{
	remain_size = 0;
}

void Session::start()
{
	doRead();		// 수신하기를 시작한다.

	// 로그인 패킷 - id를 보내준다.
	SC_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGIN;
	p.player_id = my_id;
	sendPacket(&p);

	// 나의 접속을 모든 플레이어에게 알린다.

	// 접속된 모든 플레이어를 나에게 알린다.

	// Todo : 최초 접속시 할 일
}

void Session::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite의 람다 함수에서 해제해 줄 것이다.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

void Session::doRead()
{
	auto self(shared_from_this());
	socket.async_read_some(asio::buffer(read_buffer),
		[this, self](asio::error_code ec, std::size_t length)
		{
			if (ec) {		// 실패했을 때
				if (ec.value() == asio::error::operation_aborted)
					return;
				std::cout << "Receive Error on Session[" << my_id << "]: [" << ec << "]: " << ec.message() << std::endl;
				players[my_id] = nullptr;													// 나를 제거한다.
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

void Session::doWrite(unsigned char* packet, std::size_t length)
{
	auto self(shared_from_this());
	// 함수의 첫번째 인자로 들어가는 buffer는 비동기 작업 동안 메모리가 소멸되지 않도록 메모리를 프로그램이 관리해 줘야 한다.
	socket.async_write_some(asio::buffer(packet, length),
		[this, self, packet, length](asio::error_code ec, std::size_t bytes_transferred)
		{
			// shared_ptr인 self는 이 콜백이 호출 및 리턴 될 때까지 존재한다. -> this를 참조하고 있기 때문에 필요하다. -> use_count 올려준다.
			// 람다 함수에서 값으로 캡처를 했기 때문이다. -> 만약 현재 객체를 참조하는 곳이 없어도, 생명 주기가 현재 콜백의 끝까지 연장된다.
			if (!ec)		// send 완료 시
			{
				if (length != bytes_transferred) {
					std::cout << "Incomplete Send occured on session[" << my_id << "]. This session should be closed.\n";
				}
				delete[] packet;		// 작업이 완료되었으므로 더이상 필요없고, 놔두면 Leak이다.	// 소멸자가 없는 객체는 delete, delete[] 상관없음
			}
		});
}

void Session::processPacket(unsigned char* packet)
{
	// Todo : 수신 패킷 처리

	switch (packet[1])
	{
	case CS_KEY_EVENT:
		std::cout << "Key Event 수신, ID: " << my_id << std::endl;
		break;

	default:
		std::cout << "Type Error: " << static_cast<int>(packet[1]) << " Type is invalid, player id [" << my_id << "]\n";
		break;
	}
}
