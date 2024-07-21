#include "ArcherObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>
#include <list>

ArcherATTACKState::ArcherATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArcherATTACKState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	{
		auto pos = player.getPosition();
		auto dir = player.getLook();

		int newID = dynamic_cast<ArcherObject*>(&player)->arrowID++;
		dynamic_cast<ArcherObject*>(&player)->arrowObjects.try_emplace(newID, pos, dir);

		SC_ADD_ARROW_PACKET p;
		p.size = sizeof(p);
		p.type = SC_ADD_ARROW;
		p.arrow_id = newID;
		p.pos_x = pos.x; p.pos_z = pos.z;
		p.dir_x = dir.x; p.dir_z = dir.z;

		// ��� �÷��̾�� ȭ�� �߰� ����� ����
		for (auto& a : player.parentRoom->sessions) {
			std::shared_ptr<Session> session = a.load();
			if (Room::isValidSession(session))
				session->sendPacket(&p);
		}
	}

	StateMachine::enter();
}

void ArcherATTACKState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
		// �������� State ����
		player.changeIDLEState();
	}

	StateMachine::update(elapsedTime);
}

void ArcherATTACKState::exit()
{
	StateMachine::exit();
}

ArcherSKILLState::ArcherSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArcherSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	dynamic_cast<ArcherObject*>(&player)->archerEffects.push_back(ArcherObject::ArcherEffect{ player.getPosition(), player.getLook(), -0.375f });

	StateMachine::enter();
}

void ArcherSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
		// �������� State ����
		player.changeIDLEState();
	}

	StateMachine::update(elapsedTime);
}

void ArcherSKILLState::exit()
{
	StateMachine::exit();
}

ArcherObject::Arrow::Arrow()
{
	std::cerr << "ȣ��Ǹ� �ȵ�!" << std::endl;
	exit(-1);
}

ArcherObject::Arrow::Arrow(glm::vec3 pos, glm::vec3 dir)
	: pos{ pos }, dir{ dir }
{
	spawnTime = std::chrono::steady_clock::now();
}

// static ���� (ȭ���� Ŭ�� ���忡�� �ϳ��� �ڷᱸ���� ���ֵȴ�)
int ArcherObject::arrowID;

ArcherObject::ArcherObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

ArcherObject::~ArcherObject()
{
	// �ü��� �Ҹ�� ���� ȭ�쵵 ���� �Ҹ�ȴ�
	for (auto& arr : arrowObjects) {
		SC_REMOVE_ARROW_PACKET rp;
		rp.size = sizeof(rp);
		rp.type = SC_REMOVE_ARROW;
		rp.arrow_id = arr.first;
		for (auto& a : parentRoom->sessions) {
			std::shared_ptr<Session> session = a.load();
			if (Room::isValidSession(session))
				session->sendPacket(&rp);
		}
	}
}

void ArcherObject::initialize()
{
}

bool ArcherObject::update(float elapsedTime)
{
	// ȭ�� ���� ������Ʈ
	{
		std::list<int> removeArrowList;
		for (auto& arr : arrowObjects) {
			auto nowTime = std::chrono::steady_clock::now();
			// 3�ʰ� ����Ǿ����� ����
			if (arr.second.spawnTime + std::chrono::seconds(1) <= nowTime) {
				removeArrowList.push_back(arr.first);
			}
			// �װ� �ƴϰ�, 0.375�ʰ� �������� ��ġ ������Ʈ
			else if (arr.second.spawnTime + std::chrono::milliseconds(375) <= nowTime) {
				// ȭ�� ��ġ ������Ʈ
				arr.second.pos += arr.second.dir * 20.f * elapsedTime;		// Ŭ��� ����ȭ �Ǿ�� �Ѵ�
				// ȭ��� ������ �浹 �˻�
				for (auto& m : parentRoom->monsters) {
					BoundingBox arrBox;
					arrBox.setBound(1.f, 0.1f, arr.second.pos.z + 0.5f, arr.second.pos.z - 0.5f, arr.second.pos.x - 0.5f, arr.second.pos.x + 0.5f);
					if (arrBox.isCollide(m.second->getBoundingBox())) {
						m.second->onHit(*this, 100);
						removeArrowList.push_back(arr.first);
					}
				}
				{	// ȭ��� ������ �浹 �˻�
					BoundingBox arrBox;
					arrBox.setBound(1.f, 0.1f, arr.second.pos.z + 0.5f, arr.second.pos.z - 0.5f, arr.second.pos.x - 0.5f, arr.second.pos.x + 0.5f);
					if (arrBox.isCollide(parentRoom->boss->getBoundingBox())) {
						parentRoom->boss->onHit(*this, 100);
						removeArrowList.push_back(arr.first);
					}
				}
				// ��� �÷��̾�� ȭ���� �� ��ġ ������Ʈ
				SC_MOVE_ARROW_PACKET p;
				p.size = sizeof(p);
				p.type = SC_MOVE_ARROW;
				p.arrow_id = arr.first;
				p.pos_x = arr.second.pos.x;
				p.pos_z = arr.second.pos.z;
				for (auto& a : parentRoom->sessions) {
					std::shared_ptr<Session> session = a.load();
					if (Room::isValidSession(session))
						session->sendPacket(&p);
				}
			}
		}
		// ���ŵ� ȭ�� ó��
		for (auto arr : removeArrowList) {
			SC_REMOVE_ARROW_PACKET rp;
			rp.size = sizeof(rp);
			rp.type = SC_REMOVE_ARROW;
			rp.arrow_id = arr;
			for (auto& a : parentRoom->sessions) {
				std::shared_ptr<Session> session = a.load();
				if (Room::isValidSession(session))
					session->sendPacket(&rp);
			}
			arrowObjects.erase(arr);
		}
	}
	// ��ų ���� ������Ʈ
	{
		// ������Ʈ
		for (auto& aEffect : archerEffects) {
			aEffect.accumTime += elapsedTime;
			if (aEffect.accumTime > 0.f) {
				// �̵�
				aEffect.pos += aEffect.dir * 12.f * elapsedTime;		// Ŭ��� ���� ��
				// �浹�˻�
				// ���Ϳ� �浹 �˻�
				for (auto& m : parentRoom->monsters) {
					if (0 != aEffect.attackedObject.count(m.second.get()))		// �̹� ������ ������ ������ �Ѿ��
						continue;
					BoundingBox boundingBox;
					boundingBox.setBound(1.f, 0.1f, aEffect.pos.z + 0.5f, aEffect.pos.z - 0.5f, aEffect.pos.x - 0.5f, aEffect.pos.x + 0.5f);
					if (boundingBox.isCollide(m.second->getBoundingBox())) {
						m.second->onHit(*this, 100);					// �浹�̸� �˷��ֱ�
						aEffect.attackedObject.insert(m.second.get());			// �ѹ��� ������ ������ �Ѵ�
						std::cout << m.first << ": ���� ���ݹ���" << std::endl;
					}
				}
				// ������ �浹 �˻�
				if (0 == aEffect.attackedObject.count(parentRoom->boss.get())) {	// ������ ������ ������� �Ѵ�
					BoundingBox boundingBox;
					boundingBox.setBound(1.f, 0.1f, aEffect.pos.z + 0.5f, aEffect.pos.z - 0.5f, aEffect.pos.x - 0.5f, aEffect.pos.x + 0.5f);
					if (boundingBox.isCollide(parentRoom->boss->getBoundingBox())) {
						parentRoom->boss->onHit(*this, 100);					// �浹�̸� �˷��ֱ�
						aEffect.attackedObject.insert(parentRoom->boss.get());			// �ѹ��� ������ ������ �Ѵ�
					}
				}
			}
		}
		// ����
		std::list<std::vector<ArcherObject::ArcherEffect>::iterator> deleteEffects;
		for (auto itr = archerEffects.begin(); itr != archerEffects.end(); ++itr) {
			if (itr->accumTime >= 0.625f)		// �ð� �����
				deleteEffects.emplace_back(itr);
		}
		for (const auto& itr : deleteEffects) {
			archerEffects.erase(itr);
		}
	}

	return PlayerObject::update(elapsedTime);
}

void ArcherObject::release()
{
}

void ArcherObject::onHit(const GameObjectBase& other, int damage)
{
}

void ArcherObject::changeATTACKState()
{
	nextState = std::make_unique<ArcherATTACKState>(*this);
}

void ArcherObject::changeSKILLState()
{
	nextState = std::make_unique<ArcherSKILLState>(*this);
}
