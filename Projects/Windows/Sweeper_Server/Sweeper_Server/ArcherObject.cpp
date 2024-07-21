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

		// 모든 플레이어에게 화살 추가 명령을 보냄
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
		// 끝났으면 State 변경
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
		// 끝났으면 State 변경
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
	std::cerr << "호출되면 안됨!" << std::endl;
	exit(-1);
}

ArcherObject::Arrow::Arrow(glm::vec3 pos, glm::vec3 dir)
	: pos{ pos }, dir{ dir }
{
	spawnTime = std::chrono::steady_clock::now();
}

// static 변수 (화살은 클라 입장에서 하나의 자료구조로 간주된다)
int ArcherObject::arrowID;

ArcherObject::ArcherObject(Room* parentRoom, int p_id)
	: PlayerObject{ parentRoom, p_id }
{
}

ArcherObject::~ArcherObject()
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

void ArcherObject::initialize()
{
}

bool ArcherObject::update(float elapsedTime)
{
	// 화살 관련 업데이트
	{
		std::list<int> removeArrowList;
		for (auto& arr : arrowObjects) {
			auto nowTime = std::chrono::steady_clock::now();
			// 3초가 경과되었으면 제거
			if (arr.second.spawnTime + std::chrono::seconds(1) <= nowTime) {
				removeArrowList.push_back(arr.first);
			}
			// 그게 아니고, 0.375초가 지났으면 위치 업데이트
			else if (arr.second.spawnTime + std::chrono::milliseconds(375) <= nowTime) {
				// 화살 위치 업데이트
				arr.second.pos += arr.second.dir * 20.f * elapsedTime;		// 클라와 동기화 되어야 한다
				// 화살과 몬스터의 충돌 검사
				for (auto& m : parentRoom->monsters) {
					BoundingBox arrBox;
					arrBox.setBound(1.f, 0.1f, arr.second.pos.z + 0.5f, arr.second.pos.z - 0.5f, arr.second.pos.x - 0.5f, arr.second.pos.x + 0.5f);
					if (arrBox.isCollide(m.second->getBoundingBox())) {
						m.second->onHit(*this, 100);
						removeArrowList.push_back(arr.first);
					}
				}
				{	// 화살과 보스의 충돌 검사
					BoundingBox arrBox;
					arrBox.setBound(1.f, 0.1f, arr.second.pos.z + 0.5f, arr.second.pos.z - 0.5f, arr.second.pos.x - 0.5f, arr.second.pos.x + 0.5f);
					if (arrBox.isCollide(parentRoom->boss->getBoundingBox())) {
						parentRoom->boss->onHit(*this, 100);
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
	// 스킬 관련 업데이트
	{
		// 업데이트
		for (auto& aEffect : archerEffects) {
			aEffect.accumTime += elapsedTime;
			if (aEffect.accumTime > 0.f) {
				// 이동
				aEffect.pos += aEffect.dir * 12.f * elapsedTime;		// 클라와 같은 값
				// 충돌검사
				// 몬스터와 충돌 검사
				for (auto& m : parentRoom->monsters) {
					if (0 != aEffect.attackedObject.count(m.second.get()))		// 이미 이전에 공격을 했으면 넘어간다
						continue;
					BoundingBox boundingBox;
					boundingBox.setBound(1.f, 0.1f, aEffect.pos.z + 0.5f, aEffect.pos.z - 0.5f, aEffect.pos.x - 0.5f, aEffect.pos.x + 0.5f);
					if (boundingBox.isCollide(m.second->getBoundingBox())) {
						m.second->onHit(*this, 100);					// 충돌이면 알려주기
						aEffect.attackedObject.insert(m.second.get());			// 한번만 공격이 들어가도록 한다
						std::cout << m.first << ": 몬스터 공격받음" << std::endl;
					}
				}
				// 보스와 충돌 검사
				if (0 == aEffect.attackedObject.count(parentRoom->boss.get())) {	// 이전에 공격이 없었어야 한다
					BoundingBox boundingBox;
					boundingBox.setBound(1.f, 0.1f, aEffect.pos.z + 0.5f, aEffect.pos.z - 0.5f, aEffect.pos.x - 0.5f, aEffect.pos.x + 0.5f);
					if (boundingBox.isCollide(parentRoom->boss->getBoundingBox())) {
						parentRoom->boss->onHit(*this, 100);					// 충돌이면 알려주기
						aEffect.attackedObject.insert(parentRoom->boss.get());			// 한번만 공격이 들어가도록 한다
					}
				}
			}
		}
		// 제거
		std::list<std::vector<ArcherObject::ArcherEffect>::iterator> deleteEffects;
		for (auto itr = archerEffects.begin(); itr != archerEffects.end(); ++itr) {
			if (itr->accumTime >= 0.625f)		// 시간 경과시
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
