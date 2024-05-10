#include "Room.h"

#include "Session.h"
#include "MonsterObject.h"

#include <iostream>

Room::Room(asio::io_context& io_context, int room_id)
	: io_context{ io_context }, monster_ids{ 0 }
{
	this->room_id = room_id;

	// 버섯 생성
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
	room_mutex.lock();					// 락, read 하면서 동시 접근 가능성이 있다.
	for (int i = 0; i < 4; ++i) {
		if (!sessions[i]) {
			sessions[i] = session;

			// 방에 추가 완료되었으니, 시작해준다.
			sessions[i]->start(this, i);		// 플레이어가 속한 방과 플레이어 아이디 지정

			room_mutex.unlock();		// 리턴 전 언락
			return;
		}
	}
	room_mutex.unlock();				// 여기서도 언락
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
	std::lock_guard<std::mutex> l{ room_mutex };		// 플레이어 삽입 / 삭제 주의
	for (auto& s : sessions) {
		if (isValidSession(s))
			s->update(elapsedTime);
	}

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
			for (auto& s : sessions) {		// 모든 세션에게 변화된 몬스터 위치 정보를 보내준다
				if (isValidSession(s))
					s->sendPacket(&p);
			}
		}
	}
}

bool Room::isValidSession(const std::shared_ptr<Session>& session)
{
	return session and session->player;		// 세션이 접속된 상태이면서, 로그인 패킷이 날라왔는지
}
