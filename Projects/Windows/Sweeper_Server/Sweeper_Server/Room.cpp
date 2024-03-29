#include "Room.h"

#include "Session.h"

#include <iostream>

Room::Room(int room_id)
{
	this->room_id = room_id;
}

void Room::addSession(std::shared_ptr<Session> session)
{
	room_mutex.lock();					// ��, read �ϸ鼭 ���� ���� ���ɼ��� �ִ�.
	for (int i = 0; i < 4; ++i) {
		if (!sessions[i]) {
			sessions[i] = session;

			// �濡 �߰� �Ϸ�Ǿ�����, �������ش�.
			sessions[i]->start(this, i);		// �÷��̾ ���� ��� �÷��̾� ���̵� ����

			room_mutex.unlock();		// ���� �� ���
			return;
		}
	}
	room_mutex.unlock();				// ���⼭�� ���
	// ���� �� á�� �� �̰��� ����ȴ�.
	std::cerr << "addPlayer Error!!" << std::endl;
	{
		SC_LOGIN_FAIL_PACKET p;
		p.size = sizeof(p);
		p.type = SC_LOGIN_FAIL;
		session->sendPacket(&p);	// session�� �� �Ҹ�
	}
}

void Room::update(float elapsedTime)
{
	std::lock_guard<std::mutex> l{ room_mutex };		// �÷��̾� ���� / ���� ����
	for (auto& s : sessions) {
		if (s) {
			s->update(elapsedTime);
		}
	}
}
