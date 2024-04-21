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
