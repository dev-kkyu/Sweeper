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
	// update ȣ�� �� lock�� �ɷ��ִ�.
	if (hp <= 0) {		// �׾��ٸ�..
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
					parentRoom->room_mutex.lock();
					for (auto& s : parentRoom->sessions) {	// ��� �÷��̾�� ��ü�� ������ ���� ���
						if (Room::isValidSession(s))
							s->sendPacket(&p);
					}
					parentRoom->room_mutex.unlock();
				}
			});
		return false;
	}

	// ���ݹް� ���� ���� ���� ó���� ���� �ʴ´�
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
		if (Room::isValidSession(parentRoom->sessions[i])) {		// Todo: 0�� �÷��̾� ���, ���� ����� �÷��̾�� ������ �ؾ��Ѵ�
			auto playerPos = parentRoom->sessions[i]->player->getPosition();
			auto myPos = getPosition();
			float dist2 = (myPos.x - playerPos.x) * (myPos.x - playerPos.x) + (myPos.z - playerPos.z) * (myPos.z - playerPos.z);
			float targetDist2 = 3.f * 3.f;
			if (dist2 <= std::numeric_limits<float>::epsilon())	// �Ÿ��� �ʹ� ������ ó������ �ʴ´�, Todo: ���߿� �����Ҽ��� �ִ�
				return false;
			if (dist2 <= targetDist2) {		// 3.f �Ÿ� ���� ������
				auto newDir = playerPos - myPos;
				newDir.y = 0.f;
				setLook(newDir);
				moveForward(2.f * elapsedTime);		// �ʴ� 2m �Ÿ��� �÷��̾ ���� ����

				auto bef_state = state;
				// �⺻�� Move
				state = MONSTER_STATE::MOVE;

				// �̵� �� �浹ó��
				// �÷��̾�� �浹
				if (isCollide(*parentRoom->sessions[i]->player)) {
					// �浹 �����̸� attack
					state = MONSTER_STATE::ATTACK;
					// �з����� �ϱ�
					move(-newDir, 2.f * elapsedTime);
				}

				// ���¿� ��ȭ�� �������� �����ֱ�
				if (bef_state != state) {
					sendMonsterStatePacket();
				}

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

				return true;
			}
		}
	}

	// Move �����ε� Ÿ�� �÷��̾ ���ٸ�
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
	// player�� update���� ȣ��� �����̹Ƿ�, ��X (������Ʈ ���� �� �ɱ� ����)
	{
		hp -= 100;
		attackBeginTime = std::chrono::steady_clock::now();
		state = MONSTER_STATE::HIT;
		sendMonsterStatePacket();

		auto newDir = other.getPosition() - getPosition();
		newDir.y = 0.f;
		setLook(newDir);		// �÷��̾� ��������, look�� �ٲ��ش�

		SC_MONSTER_LOOK_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MONSTER_LOOK;
		p.monster_id = my_id;
		p.dir_x = newDir.x;
		p.dir_z = newDir.z;

		for (auto& s : parentRoom->sessions) {	// ��� �÷��̾�� ����� ������  Look�� �����ش�.
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
