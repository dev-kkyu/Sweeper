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

		// 모든 플레이어에게 화살 추가 명령을 보냄
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
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// 충돌
				m.second->onHit(player);
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
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
		// 끝났으면 State 변경
		player.changeIDLEState();
	}
	else if (now_time > stateBeginTime + std::chrono::milliseconds{ 200 }) {
		// 충돌검사
		auto myPos = player.getPosition();
		myPos += player.getLook();	// 칼 범위를 플레이어 앞쪽으로 세팅해준다.
		for (auto& m : player.parentRoom->monsters) {
			auto monPos = m.second->getPosition();
			float dist2 = (myPos.x - monPos.x) * (myPos.x - monPos.x) + (myPos.z - monPos.z) * (myPos.z - monPos.z);
			if (dist2 <= 1.5f) {	// 충돌
				m.second->onHit(player);
				std::cout << m.first << ": 몬스터 공격받음" << std::endl;
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
	std::cerr << "호출되면 안됨!" << std::endl;
}

ArchorObject::Arrow::Arrow(glm::vec3 pos, glm::vec3 dir)
	: pos{ pos }, dir{ dir }
{
	spawnTime = std::chrono::steady_clock::now();
}

// static 변수 (화살은 클라 입장에서 하나의 자료구조로 간주된다)
int ArchorObject::arrowID;

ArchorObject::ArchorObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

ArchorObject::~ArchorObject()
{
	// 궁수가 소멸될 때는 화살도 같이 소멸된다
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
	// 화살 관련 업데이트
	{
		std::list<int> removeArrowList;
		for (auto& arr : arrowObjects) {
			auto nowTime = std::chrono::steady_clock::now();
			// 3초가 경과되었으면 제거
			if (arr.second.spawnTime + std::chrono::seconds(3) <= nowTime) {
				removeArrowList.push_back(arr.first);
			}
			// 그게 아니고, 0.375초가 지났으면 위치 업데이트
			else if (arr.second.spawnTime + std::chrono::milliseconds(375) <= nowTime) {
				// 화살 위치 업데이트
				arr.second.pos += arr.second.dir * 15.f * elapsedTime;		// 클라와 동기화 되어야 한다
				// 화살과 몬스터의 충돌 검사
				for (auto& m : parentRoom->monsters) {
					BoundingBox arrBox;
					arrBox.setBound(1.f, 0.1f, arr.second.pos.z + 0.5f, arr.second.pos.z - 0.5f, arr.second.pos.x - 0.5f, arr.second.pos.x + 0.5f);
					if (arrBox.isCollide(m.second->getBoundingBox())) {
						m.second->onHit(*this);
						removeArrowList.push_back(arr.first);
					}
				}
				// 모든 플레이어에게 화살의 새 위치 업데이트
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
		// 제거될 화살 처리
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
