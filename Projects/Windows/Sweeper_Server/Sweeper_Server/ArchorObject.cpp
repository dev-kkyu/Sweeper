#include "ArchorObject.h"

#include "Room.h"
#include "Session.h"
#include "MonsterObject.h"
#include <iostream>
#include <list>

ArchorAttackState::ArchorAttackState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArchorAttackState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	{
		auto pos = player.getPosition();
		auto dir = player.getLook();

		int newID = dynamic_cast<ArchorObject*>(&player)->arrowID++;
		dynamic_cast<ArchorObject*>(&player)->arrowObjects.try_emplace(newID, pos, dir);

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

void ArchorAttackState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
		// �������� State ����
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// �浹�˻�
		auto myPos = player.getPosition();
		myPos += player.getLook();	// Į ������ �÷��̾� �������� �������ش�.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// �浹
				m.second->onHit(player);
				std::cout << m.first << ": ���� ���ݹ���" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void ArchorAttackState::exit()
{
	StateMachine::exit();
}

ArchorSKILLState::ArchorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArchorSKILLState::enter()
{
	stateBeginTime = std::chrono::steady_clock::now();

	StateMachine::enter();
}

void ArchorSKILLState::update(float elapsedTime)
{
	auto now_time = std::chrono::steady_clock::now();
	if (now_time > stateBeginTime + std::chrono::milliseconds{ 560 }) {
		// �������� State ����
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// �浹�˻�
		auto myPos = player.getPosition();
		myPos += player.getLook();	// Į ������ �÷��̾� �������� �������ش�.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// �浹
				m.second->onHit(player);
				std::cout << m.first << ": ���� ���ݹ���" << std::endl;
			}
		}
	}

	StateMachine::update(elapsedTime);
}

void ArchorSKILLState::exit()
{
	StateMachine::exit();
}

ArchorObject::Arrow::Arrow()
{
	std::cerr << "ȣ��Ǹ� �ȵ�!" << std::endl;
}

ArchorObject::Arrow::Arrow(glm::vec3 pos, glm::vec3 dir)
	: pos{ pos }, dir{ dir }
{
	spawnTime = std::chrono::steady_clock::now();
}

// static ���� (ȭ���� Ŭ�� ���忡�� �ϳ��� �ڷᱸ���� ���ֵȴ�)
int ArchorObject::arrowID;

ArchorObject::ArchorObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

ArchorObject::~ArchorObject()
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

void ArchorObject::initialize()
{
}

bool ArchorObject::update(float elapsedTime)
{
	// ȭ�� ���� ������Ʈ
	{
		SC_MOVE_ARROW_PACKET p;
		p.size = sizeof(p);
		p.type = SC_MOVE_ARROW;

		std::list<int> removeArrowList;
		for (auto& arr : arrowObjects) {
			// 3�ʰ� ����Ǿ����� ����
			if (arr.second.spawnTime + std::chrono::seconds(3) <= std::chrono::steady_clock::now()) {
				SC_REMOVE_ARROW_PACKET rp;
				rp.size = sizeof(rp);
				rp.type = SC_REMOVE_ARROW;
				rp.arrow_id = arr.first;
				for (auto& a : parentRoom->sessions) {
					std::shared_ptr<Session> session = a.load();
					if (Room::isValidSession(session))
						session->sendPacket(&rp);
				}
				removeArrowList.push_back(arr.first);
			}
			// �װ� �ƴ϶�� ��ġ ������Ʈ
			else {
				// ȭ�� ��ġ ������Ʈ
				arr.second.pos += arr.second.dir * 15.f * elapsedTime;		// Ŭ��� ����ȭ �Ǿ�� �Ѵ�
				// ��� �÷��̾�� ȭ���� �� ��ġ ������Ʈ
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
		for (auto arr : removeArrowList) {
			arrowObjects.erase(arr);
		}
	}

	return PlayerObject::update(elapsedTime);
}

void ArchorObject::release()
{
}

void ArchorObject::onHit(const GameObjectBase& other)
{
}

void ArchorObject::changeATTACKState()
{
	nextState = std::make_unique<ArchorAttackState>(*this);
}

void ArchorObject::changeSKILLState()
{
	nextState = std::make_unique<ArchorSKILLState>(*this);
}
