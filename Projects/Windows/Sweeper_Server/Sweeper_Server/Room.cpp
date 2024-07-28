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

	// 버섯 생성
	// 생성자는 싱글 스레드에서 호출 (락 X)
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
		// 유니티의 왼손 좌표계와 호환되도록 바꿔준다.
		monsters[monster_ids]->setPosition({ -info.posX, info.posY, info.posZ });
		monsters[monster_ids]->rotate(-info.rotationY);
		++monster_ids;
	}

	boss = std::make_unique<BossObject>(this);

}

void Room::addSession(std::shared_ptr<Session> session)
{
	// addSession은 싱글스레드에서 동작
	for (int i = 0; i < 4; ++i) {
		if (nullptr == sessions[i].load()) {			// 아직 생성되지 않았다면

			sessions[i].store(session);		// 저장해 주고

			// 방에 추가 완료되었으니, 시작해준다.
			session->start(this, i);		// 플레이어가 속한 방과 플레이어 아이디 지정

			return;
		}
	}
	// 방이 꽉 찼을 때 이곳이 실행된다.
	std::cerr << "addPlayer Error!!" << std::endl;
	{
		SC_LOGIN_FAIL_PACKET p;
		p.size = sizeof(p);
		p.type = SC_LOGIN_FAIL;
		session->sendPacket(&p);	// session은 곧 소멸
	}
}

void Room::update(float elapsedTime)
{
	for (auto& a : sessions) {
		std::shared_ptr<Session> session = a.load();
		if (isValidSession(session))
			session->update(elapsedTime);
	}

	// 삭제가 예약된 몬스터가 있다면 업데이트 전에 제거해 준다.
	if (not reserved_monster_ids.empty()) {
		std::lock_guard<std::mutex> ml{ monster_mutex };	// 컨테이너 락
		for (const int id : reserved_monster_ids)
			monsters.erase(id);
	}
	reserved_monster_ids.clear();

	// 몬스터 업데이트
	// 삽입/삭제는 업데이트에서만 이루어 지므로, 여기선 Data Race가 아니다 (싱글 스레드)
	for (auto& m : monsters) {
		if (m.second->update(elapsedTime)) {			// 변화가 있을 시 정보를 보내준다
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
			for (auto& a : sessions) {		// 모든 세션에게 변화된 몬스터 위치 정보를 보내준다
				std::shared_ptr<Session> session = a.load();
				if (isValidSession(session))
					session->sendPacket(&p);
			}
		}
	}

	// 보스 업데이트
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
		for (auto& a : sessions) {		// 모든 세션에게 변화된 보스 위치 정보를 보내준다
			std::shared_ptr<Session> session = a.load();
			if (isValidSession(session))
				session->sendPacket(&p);
		}
	}
}

bool Room::isValidSession(const std::shared_ptr<Session>& session)
{
	return session and session->player;		// 세션이 접속된 상태이면서, 로그인 패킷이 날라왔는지
}

bool Room::isValidPlayer(const std::shared_ptr<Session>& session)
{
	return isValidSession(session) and session->player->getPlayerState() != PLAYER_STATE::DIE;
}
