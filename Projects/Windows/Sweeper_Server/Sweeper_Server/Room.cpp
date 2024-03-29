#include "Room.h"

#include "Session.h"

#include <iostream>

Room::Room(int room_id)
{
	this->room_id = room_id;
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
		if (s) {
			s->update(elapsedTime);
		}
	}
}
