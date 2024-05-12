#include "Room.h"

#include "Session.h"
#include "MonsterObject.h"

#include <iostream>

Room::Room(asio::io_context& io_context, int room_id, const std::list<MonsterInfo>& initMonsterInfo)
	: io_context{ io_context }, monster_ids{ 0 }
{
	this->room_id = room_id;

	// ���� ����
	for (const auto& info : initMonsterInfo) {
		monsters.try_emplace(monster_ids, std::make_shared<MonsterObject>(this, monster_ids));
		// ����Ƽ�� �޼� ��ǥ��� ȣȯ�ǵ��� �ٲ��ش�.
		monsters[monster_ids]->setPosition({ -info.posX, info.posY, info.posZ });
		monsters[monster_ids]->rotate(-info.rotationY);
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

	// ������ ����� ���Ͱ� �ִٸ� ������Ʈ ���� ������ �ش�.
	for (const int id : reserved_monster_ids)
		monsters.erase(id);
	reserved_monster_ids.clear();

	// ���� ������Ʈ
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
