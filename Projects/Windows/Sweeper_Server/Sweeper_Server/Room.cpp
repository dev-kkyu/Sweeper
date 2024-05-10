#include "Room.h"

#include "Session.h"
#include "MonsterObject.h"

#include <iostream>

Room::Room(asio::io_context& io_context, int room_id)
	: io_context{ io_context }, monster_ids{ 0 }
{
	this->room_id = room_id;

	// ���� ����
	for (int i = 0; i < 100; ++i) {
		monsters.try_emplace(monster_ids, std::make_shared<MonsterObject>(this, monster_ids));
		int x = i / 10 - 5;
		int z = i % 10 - 5;
		monsters[monster_ids]->setPosition({ 70.f + (x * 5.f), 0.f, 50.f + (z * 5.f) });
		++monster_ids;
	}
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
		if (isValidSession(s))
			s->update(elapsedTime);
	}

	for (auto& m : monsters) {
		if (m.second->update(elapsedTime)) {			// ��ȭ�� ���� �� ������ �����ش�
			SC_MOVE_MONSTER_PACKET p;
			p.size = sizeof(p);
			p.type = SC_MOVE_MONSTER;
			p.monster_id = m.first;
			auto pos = m.second->getPosition();
			auto look = m.second->getLook();
			p.pos_x = pos.x;
			p.pos_z = pos.z;
			p.dir_x = look.x;
			p.dir_z = look.z;
			for (auto& s : sessions) {		// ��� ���ǿ��� ��ȭ�� ���� ��ġ ������ �����ش�
				if (isValidSession(s))
					s->sendPacket(&p);
			}
		}
	}
}

bool Room::isValidSession(const std::shared_ptr<Session>& session)
{
	return session and session->player;		// ������ ���ӵ� �����̸鼭, �α��� ��Ŷ�� ����Դ���
}
