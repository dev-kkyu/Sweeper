#include "MonsterObject.h"

#include "Room.h"
#include "Session.h"

MonsterObject::MonsterObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
	collisionRadius = 0.4f;
}

MonsterObject::~MonsterObject()
{
}

void MonsterObject::initialize()
{
}

bool MonsterObject::update(float elapsedTime)
{
	return false;
}

void MonsterObject::release()
{
}

void MonsterObject::onHit(const GameObjectBase& other)
{
	// player의 update에서 호출될 예정이므로, 락X (업데이트 전에 락 걸기 때문)
	{
		SC_MONSTER_STATE_PACKET p;
		p.monster_id = my_id;
		p.size = sizeof(p);
		p.type = SC_MONSTER_STATE;
		p.state = MONSTER_STATE::HIT;
		for (auto& s : parentRoom->sessions) {	// 모든 플레이어에게 변경된 플레이어 State를 보내준다.
			if (s)
				s->sendPacket(&p);
		}
	}

	auto timer = std::make_shared<asio::steady_timer>(parentRoom->io_context
		, std::chrono::steady_clock::now() + std::chrono::milliseconds(1300));
	auto self = shared_from_this();
	timer->async_wait(
		[timer, self, this](asio::error_code ec) {	// 캡처에 shared_ptr 넣음으로써 살려준다.
			if (self.use_count() < 2)	// 내가 삭제된 상태라면 이벤트를 보내지 않는다
				return;

			SC_MONSTER_STATE_PACKET p;
			p.monster_id = my_id;
			p.size = sizeof(p);
			p.type = SC_MONSTER_STATE;
			p.state = MONSTER_STATE::IDLE;

			parentRoom->room_mutex.lock();
			for (auto& s : parentRoom->sessions) {	// 모든 플레이어에게 변경된 플레이어 State를 보내준다.
				if (s)
					s->sendPacket(&p);
			}
			parentRoom->room_mutex.unlock();
		});
}
