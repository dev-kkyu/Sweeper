#include "Session.h"

#include "Room.h"
#include "PlayerObject.h"
#include "MonsterObject.h"

#include <iostream>

Session::Session(asio::ip::tcp::socket socket) : socket{ std::move(socket) }
{
	remain_size = 0;
}

void Session::start(Room* parentRoom, int player_id)
{
	this->parentRoom = parentRoom;
	this->player_id = player_id;

	std::cout << "[" << parentRoom->room_id << ":" << player_id << "]: 접속" << std::endl;

	doRead();		// 수신하기를 시작한다.
}

void Session::update(float elapsedTime)
{
	// 변화가 있을 때에만 데이터를 전송하는 것으로 최적화
	if (player->update(elapsedTime)) {
		// 프레임마다 내 위치를 모두에게 전송한다.
		SC_MOVE_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MOVE_PLAYER;
		p.player_id = player_id;
		auto pos = player->getPosition();
		auto look = player->getLook();
		std::tie(p.pos_x, p.pos_y, p.pos_z) = std::tie(pos.x, pos.y, pos.z);
		p.dir_x = look.x; p.dir_z = look.z;

		// 내 위치를 모두에게 보낸다.
		for (auto& a : parentRoom->sessions) {
			std::shared_ptr<Session> session = a.load();
			if (Room::isValidSession(session))
				session->sendPacket(&p);
		}
	}
}

void Session::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite의 람다 함수에서 해제해 줄 것이다.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

bool Session::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN: {	// 클라에서 플레이어를 선택해 주면
		{
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
			player_type = p->player_type;

			// 플레이어 생성, 추후 타입에 따른 다른 자식클래스로 생성
			player = std::make_shared<PlayerObject>(parentRoom, player_id);
		}
		// 로그인 패킷 - id를 보내준다.
		{
			SC_LOGIN_INFO_PACKET p;
			p.size = sizeof(p);
			p.type = SC_LOGIN_INFO;
			p.room_id = parentRoom->room_id;
			p.player_id = this->player_id;
			auto pos = player->getPosition();
			auto dir = player->getLook();
			p.pos_x = pos.x;
			p.pos_z = pos.z;
			p.dir_x = dir.x;
			p.dir_z = dir.z;
			sendPacket(&p);
			std::cout << "플레이어 [" << parentRoom->room_id << ":" << this->player_id << "] 로그인 완료\n";
		}

		// 나의 접속을 모든 플레이어에게 알린다.
		{
			SC_ADD_PLAYER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_ADD_PLAYER;
			p.player_type = player_type;
			p.player_id = this->player_id;
			auto pos = player->getPosition();
			auto dir = player->getLook();
			p.pos_x = pos.x;
			p.pos_z = pos.z;
			p.dir_x = dir.x;
			p.dir_z = dir.z;
			for (int i = 0; i < parentRoom->sessions.size(); ++i) {
				if (i == this->player_id)		// 나는 알릴 필요 없다.
					continue;
				std::shared_ptr<Session> session = parentRoom->sessions[i].load();
				if (Room::isValidSession(session)) {	// 존재하는 플레이어에게 전송
					session->sendPacket(&p);
				}
			}
		}
		// 접속된 모든 플레이어를 나에게 알린다.
		{
			SC_ADD_PLAYER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_ADD_PLAYER;
			for (int i = 0; i < parentRoom->sessions.size(); ++i) {
				if (i == this->player_id)		// 나는 알릴 필요 없다.
					continue;
				std::shared_ptr<Session> session = parentRoom->sessions[i].load();
				if (Room::isValidSession(session)) {	// 존재하는 플레이어를 전송
					p.player_id = i;
					p.player_type = session->player_type;
					auto pos = session->player->getPosition();
					auto dir = session->player->getLook();
					p.pos_x = pos.x;
					p.pos_z = pos.z;
					p.dir_x = dir.x;
					p.dir_z = dir.z;
					sendPacket(&p);
				}
			}
		}

		// 모든 몬스터 추가 패킷을 보낸다
		{
			SC_ADD_MONSTER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_ADD_MONSTER;
			// 업데이트가 아닌 부분에서 컨테이너 접근은 락이 필요하다. (삽입, 삭제 주의)
			std::lock_guard<std::mutex> ml{ parentRoom->monster_mutex };
			for (auto& m : parentRoom->monsters) {
				p.monster_id = m.first;
				auto pos = m.second->getPosition();
				auto dir = m.second->getLook();
				p.pos_x = pos.x;
				p.pos_z = pos.z;
				p.dir_x = dir.x;
				p.dir_z = dir.z;
				sendPacket(&p);
			}
		}
		break;
	}
	case CS_KEY_EVENT: {
		CS_KEY_EVENT_PACKET* p = reinterpret_cast<CS_KEY_EVENT_PACKET*>(packet);
		player->processKeyInput(p->key, p->is_pressed);
		std::cout << "Key Event 수신, ID: " << parentRoom->room_id << ":" << player_id << std::endl;

		{	// 서버로 수신된 키 이벤트를 브로드캐스트 한다. (클라이언트 위치 보정용)
			SC_CLIENT_KEY_EVENT_PACKET cp;
			cp.size = sizeof(cp);
			cp.type = SC_CLIENT_KEY_EVENT;
			cp.player_id = player_id;
			cp.key = p->key;
			cp.is_pressed = p->is_pressed;

			for (auto& a : parentRoom->sessions) {	// 모든 플레이어에게 키 이벤트를 브로드캐스트 한다
				std::shared_ptr<Session> session = a.load();
				if (Room::isValidSession(session))
					session->sendPacket(&cp);
			}
		}

		break;
	}
	default:
		std::cout << "Type Error: " << static_cast<int>(packet[1]) << " Type is invalid, player id [" << parentRoom->room_id << ":" << player_id << "]\n";

		asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
		std::string remote_ip = remote_endpoint.address().to_string();
		asio::ip::port_type remote_port = remote_endpoint.port();

		std::cout << "INFO:ERROR [" << remote_ip << ":" << remote_port << "] 연결 종료" << std::endl;

		// atomic_store
		parentRoom->sessions[player_id] = nullptr;	// doRead에서 shared_from_this 해주기 때문에 not delete
		socket.close();
		return false;
		break;
	}
	return true;
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

				asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
				std::string remote_ip = remote_endpoint.address().to_string();
				asio::ip::port_type remote_port = remote_endpoint.port();

				std::cout << "INFO:DISCONNECT [" << remote_ip << ":" << remote_port << "] 연결 해제" << std::endl;

				// atomic_store
				parentRoom->sessions[player_id] = nullptr;				// 나를 제거한다.
				// 남은 모든 플레이어에게 나의 로그아웃 전송
				{
					SC_LOGOUT_PACKET p;
					p.size = sizeof(p);
					p.type = SC_LOGOUT;
					p.player_id = player_id;
					for (int i = 0; i < parentRoom->sessions.size(); ++i) {
						std::shared_ptr<Session> session = parentRoom->sessions[i].load();
						if (Room::isValidSession(session)) {					// 존재하는 플레이어에게 전송
							session->sendPacket(&p);
						}
					}
				}

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
					if (not processPacket(remain_data))									// 만든 패킷에 대한 처리하기
						return;															// 잘못된 패킷이 오면 리턴해주기
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

