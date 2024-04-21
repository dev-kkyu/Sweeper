#include "MonsterObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

#include <limits>

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
	// update 호출 전 lock이 걸려있다.
	for (int i = 0; i < 4; ++i) {
		if (parentRoom->sessions[i]) {		// Todo: 0번 플레이어 대신, 제일 가까운 플레이어에게 가도록 해야한다
			auto playerPos = parentRoom->sessions[i]->player->getPosition();
			auto myPos = getPosition();
			float dist2 = (myPos.x - playerPos.x) * (myPos.x - playerPos.x) + (myPos.z - playerPos.z) * (myPos.z - playerPos.z);
			float targetDist2 = 3.f * 3.f;
			if (dist2 < std::numeric_limits<float>::epsilon())	// 거리가 너무 작으면 처리하지 않는다, Todo: 나중에 삭제할수도 있다
				return false;
			if (dist2 <= targetDist2) {		// 3.f 거리 내에 있으면
				auto newDir = playerPos - myPos;
				setLook(newDir);
				moveForward(2.f * elapsedTime);		// 초당 2m 거리로 플레이어를 향해 간다

				// 이동 후 충돌처리
				// 플레이어와 충돌
				if (isCollide(*parentRoom->sessions[i]->player))
					move(-newDir, 2.f * elapsedTime);

				// 타 몬스터와 충돌
				for (auto& m : parentRoom->monsters) {
					if (m.first == my_id)	// 나와의 충돌처리는 하면 안된다
						continue;
					if (isCollide(*m.second)) {
						glm::vec3 otherPos = m.second->getPosition();
						glm::vec3 dir = myPos - otherPos;
						move(dir, elapsedTime * 2.f);
					}
				}

				return true;
			}
		}
	}

	return false;
}

void MonsterObject::release()
{
}

void MonsterObject::onHit(const GameObjectBase& other)
{
	// player의 update에서 호출될 예정이므로, 락X (업데이트 전에 락 걸기 때문)
	{
		auto newDir = other.getPosition() - getPosition();
		setLook(newDir);		// 플레이어 방향으로, look을 바꿔준다

		SC_MONSTER_STATE_PACKET p1;
		p1.size = sizeof(p1);
		p1.type = SC_MONSTER_STATE;
		p1.monster_id = my_id;
		p1.state = MONSTER_STATE::HIT;

		SC_MONSTER_LOOK_PACKET p2;
		p2.size = sizeof(p2);
		p2.type = SC_MONSTER_LOOK;
		p2.monster_id = my_id;
		p2.dir_x = newDir.x;
		p2.dir_z = newDir.z;

		for (auto& s : parentRoom->sessions) {	// 모든 플레이어에게 변경된 플레이어 State와 Look을 보내준다.
			if (s) {
				s->sendPacket(&p1);
				s->sendPacket(&p2);
			}
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
