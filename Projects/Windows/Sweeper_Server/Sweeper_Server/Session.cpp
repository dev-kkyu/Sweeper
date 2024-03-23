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

	doRead();		// �����ϱ⸦ �����Ѵ�.

	// �α��� ��Ŷ - id�� �����ش�.
	{
		SC_LOGIN_PACKET p;
		p.size = sizeof(p);
		p.type = SC_LOGIN;
		p.room_id = parentRoom->room_id;
		p.player_id = this->player_id;
		sendPacket(&p);
		std::cout << "�÷��̾� [" << parentRoom->room_id << ":" << this->player_id << "] ����\n";
	}
	// ���� ������ ��� �÷��̾�� �˸���.
	{
		SC_ADD_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_PLAYER;
		p.player_id = this->player_id;
		for (int i = 0; i < parentRoom->sessions.size(); ++i) {		// Room���� �� �ɾ��ֱ� ������ ���⼭�� X
			if (i == this->player_id)		// ���� �˸� �ʿ� ����.
				continue;
			if (parentRoom->sessions[i])	// �����ϴ� �÷��̾�� ����
				parentRoom->sessions[i]->sendPacket(&p);
		}
	}
	// ���ӵ� ��� �÷��̾ ������ �˸���.
	{
		SC_ADD_PLAYER_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_PLAYER;
		for (int i = 0; i < parentRoom->sessions.size(); ++i) {		// Room���� �� �ɾ��ֱ� ������ ���⼭�� X
			if (i == this->player_id)		// ���� �˸� �ʿ� ����.
				continue;
			if (parentRoom->sessions[i]) {	// �����ϴ� �÷��̾ ����
				p.player_id = i;
				sendPacket(&p);
			}
		}
	}
}

void Session::update(float elapsedTime)
{
	player->update(elapsedTime);

	// Todo : ����ȭ �ʿ� - ��ġ�� �ȹٲ������� �������� �ʾƾ� �Ѵ�.
	// �����Ӹ��� �� ��ġ�� ��ο��� �����Ѵ�.
	SC_POSITION_PACKET p;
	p.size = sizeof(p);
	p.type = SC_POSITION;
	p.player_id = player_id;
	auto pos = player->getPosition();
	std::tie(p.x, p.y, p.z) = std::tie(pos.x, pos.y, pos.z);

	// �� ��ġ�� ��ο��� ������.	// �뿡�� update ȣ�� �� �� �ɾ��ֱ� ������ ���⼭�� ���� �ʴ´�.
	for (auto& s : parentRoom->sessions) {
		if (s)
			s->sendPacket(&p);
	}
}

void Session::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite�� ���� �Լ����� ������ �� ���̴�.
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
		std::cout << "Key Event ����, ID: " << parentRoom->room_id << ":" << player_id << std::endl;
		break;
	}
	case CS_MOVE_MOUSE: {
		{
			CS_MOVE_MOUSE_PACKET* p = reinterpret_cast<CS_MOVE_MOUSE_PACKET*>(packet);
			player->processMoveMouse(p->move_x, p->move_y);		// ���콺�� ���� ȸ������ �ش�.
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
			for (auto& s : parentRoom->sessions) {			// ��� �÷��̾�� ����� �÷��̾� Look�� �����ش�.
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
			if (ec) {		// �������� ��
				if (ec.value() == asio::error::operation_aborted)
					return;
				std::cout << "Receive Error on Session[" << parentRoom->room_id << ":" << player_id << "]: [" << ec << "]: " << ec.message() << std::endl;
				parentRoom->room_mutex.lock();
				parentRoom->sessions[player_id] = nullptr;				// ���� �����Ѵ�.
				// ���� ��� �÷��̾�� ���� �α׾ƿ� ����
				{
					SC_LOGOUT_PACKET p;
					p.size = sizeof(p);
					p.type = SC_LOGOUT;
					p.player_id = player_id;
					for (int i = 0; i < parentRoom->sessions.size(); ++i) {
						if (parentRoom->sessions[i]) {					// �����ϴ� �÷��̾�� ����
							parentRoom->sessions[i]->sendPacket(&p);
						}
					}
				}
				parentRoom->room_mutex.unlock();

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
					processPacket(remain_data);											// ���� ��Ŷ�� ���� ó���ϱ�
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

