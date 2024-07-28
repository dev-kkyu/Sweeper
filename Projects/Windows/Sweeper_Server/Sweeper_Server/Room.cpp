#include "Room.h"

#include "Session.h"
#include "PlayerObject.h"
#include "MushroomObject.h"
#include "BornDogObject.h"
#include "GoblinObject.h"
#include "BoogieObject.h"

#include <iostream>

Room::Room(asio::io_context& io_context, int room_id, const std::list<MonsterInfo>& initMonsterInfo)
	: io_context{ io_context }, monster_ids{ 0 }
{
	this->room_id = room_id;

	// ���� ����
	// �����ڴ� �̱� �����忡�� ȣ�� (�� X)
	for (const auto& info : initMonsterInfo) {
		switch (info.type)
		{
		case MONSTER_TYPE::MUSHROOM:
			monsters.try_emplace(monster_ids, std::make_shared<MushroomObject>(this, monster_ids));
			break;
		case MONSTER_TYPE::BORNDOG:
			monsters.try_emplace(monster_ids, std::make_shared<BornDogObject>(this, monster_ids));
			break;
		case MONSTER_TYPE::GOBLIN:
			monsters.try_emplace(monster_ids, std::make_shared<GoblinObject>(this, monster_ids));
			break;
		case MONSTER_TYPE::BOOGIE:
			monsters.try_emplace(monster_ids, std::make_shared<BoogieObject>(this, monster_ids));
			break;
		default:
			std::cout << "ERROR: INVALID MONSTER TYPE!" << std::endl;
			continue;
			break;
		}
		// ����Ƽ�� �޼� ��ǥ��� ȣȯ�ǵ��� �ٲ��ش�.
		monsters[monster_ids]->setPosition({ -info.posX, info.posY, info.posZ });
		monsters[monster_ids]->rotate(-info.rotationY);
		++monster_ids;
	}

	boss = std::make_unique<BossObject>(this);

}

void Room::addSession(std::shared_ptr<Session> session)
{
	// addSession�� �̱۽����忡�� ����
	for (int i = 0; i < 4; ++i) {
		if (nullptr == sessions[i].load()) {			// ���� �������� �ʾҴٸ�

			sessions[i].store(session);		// ������ �ְ�

			// �濡 �߰� �Ϸ�Ǿ�����, �������ش�.
			session->start(this, i);		// �÷��̾ ���� ��� �÷��̾� ���̵� ����

			return;
		}
	}
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
	for (auto& a : sessions) {
		std::shared_ptr<Session> session = a.load();
		if (isValidSession(session))
			session->update(elapsedTime);
	}

	// ������ ����� ���Ͱ� �ִٸ� ������Ʈ ���� ������ �ش�.
	if (not reserved_monster_ids.empty()) {
		std::lock_guard<std::mutex> ml{ monster_mutex };	// �����̳� ��
		for (const int id : reserved_monster_ids)
			monsters.erase(id);
	}
	reserved_monster_ids.clear();

	// ���� ������Ʈ
	// ����/������ ������Ʈ������ �̷�� ���Ƿ�, ���⼱ Data Race�� �ƴϴ� (�̱� ������)
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
			for (auto& a : sessions) {		// ��� ���ǿ��� ��ȭ�� ���� ��ġ ������ �����ش�
				std::shared_ptr<Session> session = a.load();
				if (isValidSession(session))
					session->sendPacket(&p);
			}
		}
	}

	// ���� ������Ʈ
	if (boss->update(elapsedTime)) {
		SC_MOVE_BOSS_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MOVE_BOSS;
		auto pos = boss->getPosition();
		auto look = boss->getLook();
		p.pos_x = pos.x;
		p.pos_z = pos.z;
		p.dir_x = look.x;
		p.dir_z = look.z;
		for (auto& a : sessions) {		// ��� ���ǿ��� ��ȭ�� ���� ��ġ ������ �����ش�
			std::shared_ptr<Session> session = a.load();
			if (isValidSession(session))
				session->sendPacket(&p);
		}
	}
}

bool Room::isValidSession(const std::shared_ptr<Session>& session)
{
	return session and session->player;		// ������ ���ӵ� �����̸鼭, �α��� ��Ŷ�� ����Դ���
}

bool Room::isValidPlayer(const std::shared_ptr<Session>& session)
{
	return isValidSession(session) and session->player->getPlayerState() != PLAYER_STATE::DIE;
}
