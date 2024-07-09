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

	std::cout << "[" << parentRoom->room_id << ":" << player_id << "]: ����" << std::endl;

	doRead();		// �����ϱ⸦ �����Ѵ�.
}

void Session::update(float elapsedTime)
{
	// ��ȭ�� ���� ������ �����͸� �����ϴ� ������ ����ȭ
	if (player->update(elapsedTime)) {
		// �����Ӹ��� �� ��ġ�� ��ο��� �����Ѵ�.
		SC_MOVE_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MOVE_PLAYER;
		p.player_id = player_id;
		auto pos = player->getPosition();
		auto look = player->getLook();
		std::tie(p.pos_x, p.pos_y, p.pos_z) = std::tie(pos.x, pos.y, pos.z);
		p.dir_x = look.x; p.dir_z = look.z;

		// �� ��ġ�� ��ο��� ������.
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
	unsigned char* buff = new unsigned char[packet_size];				// doWrite�� ���� �Լ����� ������ �� ���̴�.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

bool Session::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN: {	// Ŭ�󿡼� �÷��̾ ������ �ָ�
		{
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
			player_type = p->player_type;

			// �÷��̾� ����, ���� Ÿ�Կ� ���� �ٸ� �ڽ�Ŭ������ ����
			player = std::make_shared<PlayerObject>(parentRoom, player_id);
		}
		// �α��� ��Ŷ - id�� �����ش�.
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
			std::cout << "�÷��̾� [" << parentRoom->room_id << ":" << this->player_id << "] �α��� �Ϸ�\n";
		}

		// ���� ������ ��� �÷��̾�� �˸���.
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
				if (i == this->player_id)		// ���� �˸� �ʿ� ����.
					continue;
				std::shared_ptr<Session> session = parentRoom->sessions[i].load();
				if (Room::isValidSession(session)) {	// �����ϴ� �÷��̾�� ����
					session->sendPacket(&p);
				}
			}
		}
		// ���ӵ� ��� �÷��̾ ������ �˸���.
		{
			SC_ADD_PLAYER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_ADD_PLAYER;
			for (int i = 0; i < parentRoom->sessions.size(); ++i) {
				if (i == this->player_id)		// ���� �˸� �ʿ� ����.
					continue;
				std::shared_ptr<Session> session = parentRoom->sessions[i].load();
				if (Room::isValidSession(session)) {	// �����ϴ� �÷��̾ ����
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

		// ��� ���� �߰� ��Ŷ�� ������
		{
			SC_ADD_MONSTER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_ADD_MONSTER;
			// ������Ʈ�� �ƴ� �κп��� �����̳� ������ ���� �ʿ��ϴ�. (����, ���� ����)
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
		std::cout << "Key Event ����, ID: " << parentRoom->room_id << ":" << player_id << std::endl;

		{	// ������ ���ŵ� Ű �̺�Ʈ�� ��ε�ĳ��Ʈ �Ѵ�. (Ŭ���̾�Ʈ ��ġ ������)
			SC_CLIENT_KEY_EVENT_PACKET cp;
			cp.size = sizeof(cp);
			cp.type = SC_CLIENT_KEY_EVENT;
			cp.player_id = player_id;
			cp.key = p->key;
			cp.is_pressed = p->is_pressed;

			for (auto& a : parentRoom->sessions) {	// ��� �÷��̾�� Ű �̺�Ʈ�� ��ε�ĳ��Ʈ �Ѵ�
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

		std::cout << "INFO:ERROR [" << remote_ip << ":" << remote_port << "] ���� ����" << std::endl;

		// atomic_store
		parentRoom->sessions[player_id] = nullptr;	// doRead���� shared_from_this ���ֱ� ������ not delete
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
			if (ec) {		// �������� ��
				if (ec.value() == asio::error::operation_aborted)
					return;
				std::cout << "Receive Error on Session[" << parentRoom->room_id << ":" << player_id << "]: [" << ec << "]: " << ec.message() << std::endl;

				asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
				std::string remote_ip = remote_endpoint.address().to_string();
				asio::ip::port_type remote_port = remote_endpoint.port();

				std::cout << "INFO:DISCONNECT [" << remote_ip << ":" << remote_port << "] ���� ����" << std::endl;

				// atomic_store
				parentRoom->sessions[player_id] = nullptr;				// ���� �����Ѵ�.
				// ���� ��� �÷��̾�� ���� �α׾ƿ� ����
				{
					SC_LOGOUT_PACKET p;
					p.size = sizeof(p);
					p.type = SC_LOGOUT;
					p.player_id = player_id;
					for (int i = 0; i < parentRoom->sessions.size(); ++i) {
						std::shared_ptr<Session> session = parentRoom->sessions[i].load();
						if (Room::isValidSession(session)) {					// �����ϴ� �÷��̾�� ����
							session->sendPacket(&p);
						}
					}
				}

				return;
			}

			int received = static_cast<int>(length);
			unsigned char* ptr = read_buffer;

			// ��Ŷ ������
			while (received > 0) {
				int packet_size = ptr[0];
				if (remain_size > 0) {
					packet_size = remain_data[0];
				}

				if (remain_size + received >= packet_size) {							// ������ �ִ� ������ ũ�Ⱑ ó���� ������ ũ�� �̻��϶�
					memcpy(remain_data + remain_size, ptr, packet_size - remain_size);	// ó���� �����͸�ŭ ��Ŷ�� ������ش�.
					ptr += packet_size - remain_size;									// ������ ó���� �������� ���� ��ġ�� �̸� �̵�
					received -= packet_size - remain_size;								// ������ ���� ������ ũ�⸦ ����
					if (not processPacket(remain_data))									// ���� ��Ŷ�� ���� ó���ϱ�
						return;															// �߸��� ��Ŷ�� ���� �������ֱ�
					remain_size = 0;													// remain_size�� received�� ���յǾ��� ������ 0���� ������ش�.
				}
				else {
					memcpy(remain_data + remain_size, ptr, received);					// ������ ó���� ������ �����صα�
					remain_size += received;
					received = 0;
				}
			}

			doRead();	// ���� ���� �����͸� ��ٸ���.
		});
}

void Session::doWrite(unsigned char* packet, std::size_t length)
{
	auto self(shared_from_this());
	// �Լ��� ù��° ���ڷ� ���� buffer�� �񵿱� �۾� ���� �޸𸮰� �Ҹ���� �ʵ��� �޸𸮸� ���α׷��� ������ ��� �Ѵ�.
	socket.async_write_some(asio::buffer(packet, length),
		[this, self, packet, length](asio::error_code ec, std::size_t bytes_transferred)
		{
			// shared_ptr�� self�� �� �ݹ��� ȣ�� �� ���� �� ������ �����Ѵ�. -> this�� �����ϰ� �ֱ� ������ �ʿ��ϴ�. -> use_count �÷��ش�.
			// ���� �Լ����� ������ ĸó�� �߱� �����̴�. -> ���� ���� ��ü�� �����ϴ� ���� ���, ���� �ֱⰡ ���� �ݹ��� ������ ����ȴ�.
			if (!ec)		// send �Ϸ� ��
			{
				if (length != bytes_transferred) {
					std::cout << "Incomplete Send occured on session[" << parentRoom->room_id << ":" << player_id << "]. This session should be closed.\n";
				}
				delete[] packet;		// �۾��� �Ϸ�Ǿ����Ƿ� ���̻� �ʿ����, ���θ� Leak�̴�.	// �Ҹ��ڰ� ���� ��ü�� delete, delete[] �������
			}
		});
}

