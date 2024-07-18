#pragma once

#include <array>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <list>

#include "protocol.h"

#include "BossObject.h"

struct MonsterInfo
{
	MONSTER_TYPE type;
	float posX, posY, posZ;
	float rotationY;
};

namespace asio { class io_context; }
class MonsterObject;
class Session;
class Room
{
private:
	int monster_ids;		// ���� ������ �������ִ� ID, �Ѱ��� �þ��

public:
	int room_id = -1;

	// Ÿ�̸Ӹ� ���� io_context�� ����ִ´�
	asio::io_context& io_context;

	std::array<std::atomic<std::shared_ptr<Session>>, 4> sessions;	// �� �濡�� 4���� �ִ�

	std::mutex monster_mutex;		// �����̳ʴ� thread-safe ���� �ʴ�
	std::unordered_map<int, std::shared_ptr<MonsterObject>> monsters;
	std::list<int> reserved_monster_ids;

	std::unique_ptr<BossObject> boss;

public:
	Room(asio::io_context& io_context, int room_id, const std::list<MonsterInfo>& initMonsterInfo);
	void addSession(std::shared_ptr<Session> session);
	void update(float elapsedTime);

	static bool isValidSession(const std::shared_ptr<Session>& session);

};

