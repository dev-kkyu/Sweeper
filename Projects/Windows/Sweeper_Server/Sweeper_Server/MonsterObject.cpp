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
	// update ȣ�� �� lock�� �ɷ��ִ�.
	for (int i = 0; i < 4; ++i) {
		if (parentRoom->sessions[i]) {		// Todo: 0�� �÷��̾� ���, ���� ����� �÷��̾�� ������ �ؾ��Ѵ�
			auto playerPos = parentRoom->sessions[i]->player->getPosition();
			auto myPos = getPosition();
			float dist2 = (myPos.x - playerPos.x) * (myPos.x - playerPos.x) + (myPos.z - playerPos.z) * (myPos.z - playerPos.z);
			float targetDist2 = 3.f * 3.f;
			if (dist2 < std::numeric_limits<float>::epsilon())	// �Ÿ��� �ʹ� ������ ó������ �ʴ´�, Todo: ���߿� �����Ҽ��� �ִ�
				return false;
			if (dist2 <= targetDist2) {		// 3.f �Ÿ� ���� ������
				auto newDir = playerPos - myPos;
				setLook(newDir);
				moveForward(2.f * elapsedTime);		// �ʴ� 2m �Ÿ��� �÷��̾ ���� ����

				// �̵� �� �浹ó��
				// �÷��̾�� �浹
				if (isCollide(*parentRoom->sessions[i]->player))
					move(-newDir, 2.f * elapsedTime);

				// Ÿ ���Ϳ� �浹
				for (auto& m : parentRoom->monsters) {
					if (m.first == my_id)	// ������ �浹ó���� �ϸ� �ȵȴ�
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
	// player�� update���� ȣ��� �����̹Ƿ�, ��X (������Ʈ ���� �� �ɱ� ����)
	{
		auto newDir = other.getPosition() - getPosition();
		setLook(newDir);		// �÷��̾� ��������, look�� �ٲ��ش�

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

		for (auto& s : parentRoom->sessions) {	// ��� �÷��̾�� ����� �÷��̾� State�� Look�� �����ش�.
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
		[timer, self, this](asio::error_code ec) {	// ĸó�� shared_ptr �������ν� ����ش�.
			if (self.use_count() < 2)	// ���� ������ ���¶�� �̺�Ʈ�� ������ �ʴ´�
				return;

			SC_MONSTER_STATE_PACKET p;
			p.monster_id = my_id;
			p.size = sizeof(p);
			p.type = SC_MONSTER_STATE;
			p.state = MONSTER_STATE::IDLE;

			parentRoom->room_mutex.lock();
			for (auto& s : parentRoom->sessions) {	// ��� �÷��̾�� ����� �÷��̾� State�� �����ش�.
				if (s)
					s->sendPacket(&p);
			}
			parentRoom->room_mutex.unlock();
		});
}
