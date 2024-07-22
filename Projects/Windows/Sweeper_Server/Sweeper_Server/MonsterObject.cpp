#include "MonsterObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

#include <limits>

MonsterObject::MonsterObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
	state = MONSTER_STATE::IDLE;

	collisionRadius = 10.f;		// 자식이 항상 값을 정해줘야 함

	lastHitStateTime = std::chrono::steady_clock::now();

	hitDelayTime_ms = 10000;	// 자식이 항상 값을 정해줘야 함

	targetPlayer = -1;	// 0 ~ 3
}

MonsterObject::~MonsterObject()
{
}

void MonsterObject::initialize()
{
}

bool MonsterObject::update(float elapsedTime)
{
	if (HP <= 0) {		// 죽었다면..
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
					for (auto& a : parentRoom->sessions) {	// 모든 플레이어에게 객체를 제거할 것을 명령
						std::shared_ptr<Session> session = a.load();
						if (Room::isValidSession(session))
							session->sendPacket(&p);
					}
				}
			});
		return false;
	}

	// 상태에 따라 다른 로직
	switch (state)
	{
	case MONSTER_STATE::IDLE: {
		for (int i = 0; i < 4; ++i) {
			std::shared_ptr<Session> session = parentRoom->sessions[i].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				auto myPos = getPosition();

				// 나와 플레이어 사이의 거리
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				if (dist2 < std::numeric_limits<float>::epsilon())	// 거리가 너무 작으면 처리하지 않는다, Todo: 나중에 삭제할수도 있다
					continue;

				float targetDist2 = glm::pow(3.f, 2.f);				// 찾는 거리 3.f

				if (dist2 <= targetDist2) {							// 찾는 거리 내에 있으면
					targetPlayer = i;								// 타겟을 정해주고

					state = MONSTER_STATE::MOVE;					// 상태를 바꾼다
					sendMonsterStatePacket();

					break;
				}
			}
		}
		break;
	}
	case MONSTER_STATE::MOVE: {
		if (targetPlayer >= 0) {
			std::shared_ptr<Session> session = parentRoom->sessions[targetPlayer].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				auto myPos = getPosition();

				// 나와 플레이어 사이의 거리
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				float targetDist2 = glm::pow(3.f, 2.f);				// 찾는 거리 3.f
				if (dist2 > targetDist2) {			// 거리가 멀어지면
					state = MONSTER_STATE::IDLE;	// IDLE 상태로 복귀
					targetPlayer = -1;
					sendMonsterStatePacket();
				}
				else {
					// 플레이어와 충돌
					if (isCollide(*session->player)) {
						// 타겟 플레이어와 충돌이면 attack
						state = MONSTER_STATE::ATTACK;
						sendMonsterStatePacket();
					}
					else {	// 충돌이 아니면 이동
						auto newDir = playerPos - myPos;
						newDir.y = 0.f;
						setLook(newDir);
						moveForward(2.f * elapsedTime);		// 초당 2m 거리로 플레이어를 향해 간다

						// 충돌처리
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

						return true;		// 움직였으니 패킷을 보내야 한다
					}
				}
			}
			else {
				targetPlayer = -1;
				state = MONSTER_STATE::IDLE;				// 상태를 바꾼다
				sendMonsterStatePacket();
			}
		}
		else {
			state = MONSTER_STATE::IDLE;					// 상태를 바꾼다
			sendMonsterStatePacket();
		}
		break;
	}
	case MONSTER_STATE::HIT: {
		if (lastHitStateTime + std::chrono::milliseconds{ hitDelayTime_ms } < std::chrono::steady_clock::now()) {
			state = MONSTER_STATE::IDLE;
			if (targetPlayer >= 0) {			// 타겟이 있고 아직도 충돌이면 Attack
				std::shared_ptr<Session> session = parentRoom->sessions[targetPlayer].load();
				if (Room::isValidSession(session)) {
					auto playerPos = session->player->getPosition();
					auto myPos = getPosition();
					// 나와 플레이어 사이의 거리
					float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
					if (dist2 <= 1.5f) {		// 일정 거리 안이라면 바로 ATTACK
						state = MONSTER_STATE::ATTACK;
					}
				}
				else {
					targetPlayer = -1;
				}
			}
			sendMonsterStatePacket();
		}
		break;
	}
	case MONSTER_STATE::DIE: {
		// 사라지기만을 기다린다...
		break;
	}
	case MONSTER_STATE::ATTACK: {
		if (targetPlayer >= 0) {
			std::shared_ptr<Session> session = parentRoom->sessions[targetPlayer].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				auto myPos = getPosition();
				// 나와 플레이어 사이의 거리
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				if (dist2 > glm::pow(0.5f + collisionRadius, 2.f)) {							// 일정 거리 밖으로 플레이어가 움직이면
					state = MONSTER_STATE::MOVE;			// 다시 움직인다
					sendMonsterStatePacket();
				}
				else {
					// Todo: 플레이어에게 데미지를 주는 로직 추가
				}
			}
			else {
				targetPlayer = -1;
				state = MONSTER_STATE::IDLE;				// 상태를 바꾼다
				sendMonsterStatePacket();
			}
		}
		else {
			state = MONSTER_STATE::IDLE;					// 상태를 바꾼다
			sendMonsterStatePacket();
		}
		break;
	}
	default:
		break;
	}

	return false;
}

void MonsterObject::release()
{
}

void MonsterObject::onHit(const GameObjectBase& other, int damage)
{
	HP -= damage;
	lastHitStateTime = std::chrono::steady_clock::now();
	state = MONSTER_STATE::HIT;
	sendMonsterStatePacket();

	auto newDir = other.getPosition() - getPosition();
	newDir.y = 0.f;
	setLook(newDir);		// 플레이어 방향으로, look을 바꿔준다

	SC_MONSTER_LOOK_PACKET p1;
	p1.size = sizeof(p1);
	p1.type = SC_MONSTER_LOOK;
	p1.monster_id = my_id;
	p1.dir_x = newDir.x;
	p1.dir_z = newDir.z;

	SC_MONSTER_HP_PACKET p2;
	p2.size = sizeof(p2);
	p2.type = SC_MONSTER_HP;
	p2.monster_id = my_id;
	p2.hp = HP;

	for (auto& a : parentRoom->sessions) {	// 모든 플레이어에게 변경된 몬스터의 Look과 HP를 보내준다.
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session)) {
			session->sendPacket(&p1);
			session->sendPacket(&p2);
		}
	}
}

MONSTER_TYPE MonsterObject::getMonsterType() const
{
	return type;
}

void MonsterObject::sendMonsterStatePacket()
{
	SC_MONSTER_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_MONSTER_STATE;
	p.monster_id = my_id;
	p.state = state;

	for (auto& a : parentRoom->sessions) {
		std::shared_ptr<Session> session = a.load();
		if (Room::isValidSession(session))
			session->sendPacket(&p);
	}
}
