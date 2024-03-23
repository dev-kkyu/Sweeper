#include "Session.h"

#include "Room.h"
#include "PlayerObject.h"

#include <iostream>

Session::Session(asio::ip::tcp::socket socket) : socket{ std::move(socket) }
{
	remain_size = 0;

	player = std::make_shared<PlayerObject>();
}

void Session::start(Room* parentRoom, int player_id)
{
	this->parentRoom = parentRoom;
	this->player_id = player_id;

	doRead();		// 수신하기를 시작한다.

	// 로그인 패킷 - id를 보내준다.
	{
		SC_LOGIN_PACKET p;
		p.size = sizeof(p);
		p.type = SC_LOGIN;
		p.room_id = parentRoom->room_id;
		p.player_id = this->player_id;
		sendPacket(&p);
		std::cout << "플레이어 [" << parentRoom->room_id << ":" << this->player_id << "] 접속\n";
	}
	// 나의 접속을 모든 플레이어에게 알린다.
	{
		SC_ADD_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_PLAYER;
		p.player_id = this->player_id;
		for (int i = 0; i < parentRoom->sessions.size(); ++i) {		// Room에서 락 걸어주기 때문에 여기서는 X
			if (i == this->player_id)		// 나는 알릴 필요 없다.
				continue;
			if (parentRoom->sessions[i])	// 존재하는 플레이어에게 전송
				parentRoom->sessions[i]->sendPacket(&p);
		}
	}
	// 접속된 모든 플레이어를 나에게 알린다.
	{
		SC_ADD_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_PLAYER;
		for (int i = 0; i < parentRoom->sessions.size(); ++i) {		// Room에서 락 걸어주기 때문에 여기서는 X
			if (i == this->player_id)		// 나는 알릴 필요 없다.
				continue;
			if (parentRoom->sessions[i]) {	// 존재하는 플레이어를 전송
				p.player_id = i;
				sendPacket(&p);
			}
		}
	}
}

void Session::update(float elapsedTime)
{
	player->update(elapsedTime);

	// Todo : 최적화 필요 - 위치가 안바꼈을때는 전송하지 않아야 한다.
	// 프레임마다 내 위치를 모두에게 전송한다.
	SC_POSITION_PACKET p;
	p.size = sizeof(p);
	p.type = SC_POSITION;
	p.player_id = player_id;
	auto pos = player->getPosition();
	std::tie(p.x, p.y, p.z) = std::tie(pos.x, pos.y, pos.z);

	// 내 위치를 모두에게 보낸다.	// 룸에서 update 호출 시 락 걸어주기 때문에 여기서는 하지 않는다.
	for (auto& s : parentRoom->sessions) {
		if (s)
			s->sendPacket(&p);
	}
}

void Session::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite의 람다 함수에서 해제해 줄 것이다.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

void Session::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case CS_KEY_EVENT: {
		CS_KEY_EVENT_PACKET* p = reinterpret_cast<CS_KEY_EVENT_PACKET*>(packet);
		switch (p->key)
		{
		case MY_KEY_EVENT::UP:
			player->processKeyInput(KEY_UP, p->is_pressed);
			break;
		case MY_KEY_EVENT::DOWN:
			player->processKeyInput(KEY_DOWN, p->is_pressed);
			break;
		case MY_KEY_EVENT::LEFT:
			player->processKeyInput(KEY_LEFT, p->is_pressed);
			break;
		case MY_KEY_EVENT::RIGHT:
			player->processKeyInput(KEY_RIGHT, p->is_pressed);
			break;
		case MY_KEY_EVENT::SPACE:
			player->processKeyInput(KEY_SPACE, p->is_pressed);
			break;
		}
		std::cout << "Key Event 수신, ID: " << parentRoom->room_id << ":" << player_id << std::endl;
		break;
	}
	case CS_MOVE_MOUSE: {
		{
			CS_MOVE_MOUSE_PACKET* p = reinterpret_cast<CS_MOVE_MOUSE_PACKET*>(packet);
			player->processMoveMouse(p->move_x, p->move_y);		// 마우스에 따라 회전시켜 준다.
		}
		{
			auto look = player->getLook();
			SC_PLAYER_LOOK_PACKET p;
			p.size = sizeof(p);
			p.type = SC_PLAYER_LOOK;
			p.player_id = player_id;
			p.dir_x = look.x;
			p.dir_z = look.z;
			parentRoom->room_mutex.lock();
			for (auto& s : parentRoom->sessions) {			// 모든 플레이어에게 변경된 플레이어 Look을 보내준다.
				if (s)
					s->sendPacket(&p);
			}
			parentRoom->room_mutex.unlock();
		}
		break;
	}
	default:
		std::cout << "Type Error: " << static_cast<int>(packet[1]) << " Type is invalid, player id [" << parentRoom->room_id << ":" << player_id << "]\n";
		break;
	}
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
				std::cout << "Receive Error on Session[" << parentRoom->room_id << ":" << player_id << "]: [" << ec << "]: " << ec.message() << std::endl;
				parentRoom->room_mutex.lock();
				parentRoom->sessions[player_id] = nullptr;				// 나를 제거한다.
				// 남은 모든 플레이어에게 나의 로그아웃 전송
				{
					SC_LOGOUT_PACKET p;
					p.size = sizeof(p);
					p.type = SC_LOGOUT;
					p.player_id = player_id;
					for (int i = 0; i < parentRoom->sessions.size(); ++i) {
						if (parentRoom->sessions[i]) {					// 존재하는 플레이어에게 전송
							parentRoom->sessions[i]->sendPacket(&p);
						}
					}
				}
				parentRoom->room_mutex.unlock();

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
					std::cout << "Incomplete Send occured on session[" << parentRoom->room_id << ":" << player_id << "]. This session should be closed.\n";
				}
				delete[] packet;		// 작업이 완료되었으므로 더이상 필요없고, 놔두면 Leak이다.	// 소멸자가 없는 객체는 delete, delete[] 상관없음
			}
		});
}

