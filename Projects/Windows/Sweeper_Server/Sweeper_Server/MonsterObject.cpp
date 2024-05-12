#include "MonsterObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

#include <limits>

MonsterObject::MonsterObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
	state = MONSTER_STATE::IDLE;
	hp = 300;

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
	if (hp <= 0) {		// 죽었다면..
		parentRoom->reserved_monster_ids.emplace_back(my_id);		// 컨테이너에서는 삭제를 예약하여 더이상의 접근을 막는다.
		state = MONSTER_STATE::DIE;
		sendMonsterStatePacket();						// DIE 애니메이션을 재생하도록 알려줌
		auto timer = std::make_shared<asio::steady_timer>(parentRoom->io_context			// 3초 뒤에 동작하는 타이머
			, std::chrono::steady_clock::now() + std::chrono::milliseconds(3000));
		auto self = shared_from_this();
		timer->async_wait(
			[timer, self, this](asio::error_code ec) {	// 캡처에 shared_ptr 넣음으로써 살려준다.
				if (!ec) {
					SC_REMOVE_MONSTER_PACKET p;
					p.size = sizeof(p);
					p.type = SC_REMOVE_MONSTER;
					p.monster_id = my_id;
					parentRoom->room_mutex.lock();
					for (auto& s : parentRoom->sessions) {	// 모든 플레이어에게 객체를 제거할 것을 명령
						if (Room::isValidSession(s))
							s->sendPacket(&p);
					}
					parentRoom->room_mutex.unlock();
				}
			});
		return false;
	}

	// 공격받고 있을 때는 별도 처리를 하지 않는다
	if (state == MONSTER_STATE::HIT) {
		if (attackBeginTime + std::chrono::milliseconds{ 1300 } < std::chrono::steady_clock::now()) {
			state = MONSTER_STATE::IDLE;
			sendMonsterStatePacket();
		}
		else {
			return false;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (Room::isValidSession(parentRoom->sessions[i])) {		// Todo: 0번 플레이어 대신, 제일 가까운 플레이어에게 가도록 해야한다
			auto playerPos = parentRoom->sessions[i]->player->getPosition();
			auto myPos = getPosition();
			float dist2 = (myPos.x - playerPos.x) * (myPos.x - playerPos.x) + (myPos.z - playerPos.z) * (myPos.z - playerPos.z);
			float targetDist2 = 3.f * 3.f;
			if (dist2 <= std::numeric_limits<float>::epsilon())	// 거리가 너무 작으면 처리하지 않는다, Todo: 나중에 삭제할수도 있다
				return false;
			if (dist2 <= targetDist2) {		// 3.f 거리 내에 있으면
				auto newDir = playerPos - myPos;
				newDir.y = 0.f;
				setLook(newDir);
				moveForward(2.f * elapsedTime);		// 초당 2m 거리로 플레이어를 향해 간다

				auto bef_state = state;
				// 기본은 Move
				state = MONSTER_STATE::MOVE;

				// 이동 후 충돌처리
				// 플레이어와 충돌
				if (isCollide(*parentRoom->sessions[i]->player)) {
					// 충돌 상태이면 attack
					state = MONSTER_STATE::ATTACK;
					// 밀려나게 하기
					move(-newDir, 2.f * elapsedTime);
				}

				// 상태에 변화가 생겼으면 보내주기
				if (bef_state != state) {
					sendMonsterStatePacket();
				}

				// 타 몬스터와 충돌
				for (auto& m : parentRoom->monsters) {
					if (m.first == my_id)	// 나와의 충돌처리는 하면 안된다
						continue;
					if (isCollide(*m.second)) {
						glm::vec3 otherPos = m.second->getPosition();
						glm::vec3 dir = myPos - otherPos;
						dir.y = 0.f;
						move(dir, elapsedTime * 2.f);
					}
				}

				return true;
			}
		}
	}

	// Move 상태인데 타겟 플레이어가 없다면
	if (state == MONSTER_STATE::MOVE) {
		state = MONSTER_STATE::IDLE;
		sendMonsterStatePacket();
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
		hp -= 100;
		attackBeginTime = std::chrono::steady_clock::now();
		state = MONSTER_STATE::HIT;
		sendMonsterStatePacket();

		auto newDir = other.getPosition() - getPosition();
		newDir.y = 0.f;
		setLook(newDir);		// 플레이어 방향으로, look을 바꿔준다

		SC_MONSTER_LOOK_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MONSTER_LOOK;
		p.monster_id = my_id;
		p.dir_x = newDir.x;
		p.dir_z = newDir.z;

		for (auto& s : parentRoom->sessions) {	// 모든 플레이어에게 변경된 몬스터의  Look을 보내준다.
			if (Room::isValidSession(s)) {
				s->sendPacket(&p);
			}
		}
	}
}

void MonsterObject::sendMonsterStatePacket()
{
	SC_MONSTER_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_MONSTER_STATE;
	p.monster_id = my_id;
	p.state = state;

	for (auto& s : parentRoom->sessions) {
		if (Room::isValidSession(s))
			s->sendPacket(&p);
	}
}
