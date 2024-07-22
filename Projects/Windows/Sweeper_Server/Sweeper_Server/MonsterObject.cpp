#include "MonsterObject.h"

#include "Room.h"
#include "Session.h"
#include "PlayerObject.h"

#include <limits>

MonsterObject::MonsterObject(Room* parentRoom, int m_id)
	: GameObjectBase{ parentRoom, m_id }
{
	state = MONSTER_STATE::IDLE;

	collisionRadius = 10.f;		// �ڽ��� �׻� ���� ������� ��

	lastHitStateTime = std::chrono::steady_clock::now();

	hitDelayTime_ms = 10000;	// �ڽ��� �׻� ���� ������� ��

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
	if (HP <= 0) {		// �׾��ٸ�..
		parentRoom->reserved_monster_ids.emplace_back(my_id);		// �����̳ʿ����� ������ �����Ͽ� ���̻��� ������ ���´�.
		state = MONSTER_STATE::DIE;
		sendMonsterStatePacket();						// DIE �ִϸ��̼��� ����ϵ��� �˷���
		auto timer = std::make_shared<asio::steady_timer>(parentRoom->io_context			// 3�� �ڿ� �����ϴ� Ÿ�̸�
			, std::chrono::steady_clock::now() + std::chrono::milliseconds(3000));
		auto self = shared_from_this();
		timer->async_wait(
			[timer, self, this](asio::error_code ec) {	// ĸó�� shared_ptr �������ν� ����ش�.
				if (!ec) {
					SC_REMOVE_MONSTER_PACKET p;
					p.size = sizeof(p);
					p.type = SC_REMOVE_MONSTER;
					p.monster_id = my_id;
					for (auto& a : parentRoom->sessions) {	// ��� �÷��̾�� ��ü�� ������ ���� ���
						std::shared_ptr<Session> session = a.load();
						if (Room::isValidSession(session))
							session->sendPacket(&p);
					}
				}
			});
		return false;
	}

	// ���¿� ���� �ٸ� ����
	switch (state)
	{
	case MONSTER_STATE::IDLE: {
		for (int i = 0; i < 4; ++i) {
			std::shared_ptr<Session> session = parentRoom->sessions[i].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				auto myPos = getPosition();

				// ���� �÷��̾� ������ �Ÿ�
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				if (dist2 < std::numeric_limits<float>::epsilon())	// �Ÿ��� �ʹ� ������ ó������ �ʴ´�, Todo: ���߿� �����Ҽ��� �ִ�
					continue;

				float targetDist2 = glm::pow(3.f, 2.f);				// ã�� �Ÿ� 3.f

				if (dist2 <= targetDist2) {							// ã�� �Ÿ� ���� ������
					targetPlayer = i;								// Ÿ���� �����ְ�

					state = MONSTER_STATE::MOVE;					// ���¸� �ٲ۴�
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

				// ���� �÷��̾� ������ �Ÿ�
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				float targetDist2 = glm::pow(3.f, 2.f);				// ã�� �Ÿ� 3.f
				if (dist2 > targetDist2) {			// �Ÿ��� �־�����
					state = MONSTER_STATE::IDLE;	// IDLE ���·� ����
					targetPlayer = -1;
					sendMonsterStatePacket();
				}
				else {
					// �÷��̾�� �浹
					if (isCollide(*session->player)) {
						// Ÿ�� �÷��̾�� �浹�̸� attack
						state = MONSTER_STATE::ATTACK;
						sendMonsterStatePacket();
					}
					else {	// �浹�� �ƴϸ� �̵�
						auto newDir = playerPos - myPos;
						newDir.y = 0.f;
						setLook(newDir);
						moveForward(2.f * elapsedTime);		// �ʴ� 2m �Ÿ��� �÷��̾ ���� ����

						// �浹ó��
						// Ÿ ���Ϳ� �浹
						for (auto& m : parentRoom->monsters) {
							if (m.first == my_id)	// ������ �浹ó���� �ϸ� �ȵȴ�
								continue;
							if (isCollide(*m.second)) {
								glm::vec3 otherPos = m.second->getPosition();
								glm::vec3 dir = myPos - otherPos;
								dir.y = 0.f;
								move(dir, elapsedTime * 2.f);
							}
						}

						return true;		// ���������� ��Ŷ�� ������ �Ѵ�
					}
				}
			}
			else {
				targetPlayer = -1;
				state = MONSTER_STATE::IDLE;				// ���¸� �ٲ۴�
				sendMonsterStatePacket();
			}
		}
		else {
			state = MONSTER_STATE::IDLE;					// ���¸� �ٲ۴�
			sendMonsterStatePacket();
		}
		break;
	}
	case MONSTER_STATE::HIT: {
		if (lastHitStateTime + std::chrono::milliseconds{ hitDelayTime_ms } < std::chrono::steady_clock::now()) {
			state = MONSTER_STATE::IDLE;
			if (targetPlayer >= 0) {			// Ÿ���� �ְ� ������ �浹�̸� Attack
				std::shared_ptr<Session> session = parentRoom->sessions[targetPlayer].load();
				if (Room::isValidSession(session)) {
					auto playerPos = session->player->getPosition();
					auto myPos = getPosition();
					// ���� �÷��̾� ������ �Ÿ�
					float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
					if (dist2 <= 1.5f) {		// ���� �Ÿ� ���̶�� �ٷ� ATTACK
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
		// ������⸸�� ��ٸ���...
		break;
	}
	case MONSTER_STATE::ATTACK: {
		if (targetPlayer >= 0) {
			std::shared_ptr<Session> session = parentRoom->sessions[targetPlayer].load();
			if (Room::isValidSession(session)) {
				auto playerPos = session->player->getPosition();
				auto myPos = getPosition();
				// ���� �÷��̾� ������ �Ÿ�
				float dist2 = glm::pow(myPos.x - playerPos.x, 2.f) + glm::pow(myPos.z - playerPos.z, 2.f);
				if (dist2 > glm::pow(0.5f + collisionRadius, 2.f)) {							// ���� �Ÿ� ������ �÷��̾ �����̸�
					state = MONSTER_STATE::MOVE;			// �ٽ� �����δ�
					sendMonsterStatePacket();
				}
				else {
					// Todo: �÷��̾�� �������� �ִ� ���� �߰�
				}
			}
			else {
				targetPlayer = -1;
				state = MONSTER_STATE::IDLE;				// ���¸� �ٲ۴�
				sendMonsterStatePacket();
			}
		}
		else {
			state = MONSTER_STATE::IDLE;					// ���¸� �ٲ۴�
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
	setLook(newDir);		// �÷��̾� ��������, look�� �ٲ��ش�

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

	for (auto& a : parentRoom->sessions) {	// ��� �÷��̾�� ����� ������ Look�� HP�� �����ش�.
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
