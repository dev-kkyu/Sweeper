#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace asio { class io_context; }
class MonsterObject;
class Session;
class Room
{
private:
	int monster_ids;		// 몬스터 생성을 관리해주는 ID, 한개씩 늘어난다

public:
	int room_id = -1;

	// 타이머를 위해 io_context도 들고있는다
	asio::io_context& io_context;

	std::array<std::shared_ptr<Session>, 4> sessions;	// 한 방에는 4명이 있다

	std::unordered_map<int, std::shared_ptr<MonsterObject>> monsters;
	std::list<int> reserved_monster_ids;

	std::mutex room_mutex;

public:
	Room(asio::io_context& io_context, int room_id);
	void addSession(std::shared_ptr<Session> session);
	void update(float elapsedTime);

	static bool isValidSession(const std::shared_ptr<Session>& session);

};

