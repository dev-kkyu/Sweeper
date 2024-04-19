#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>

class MonsterObject;
class Session;
class Room
{
private:
	int monster_ids;		// ���� ������ �������ִ� ID, �Ѱ��� �þ��

public:
	int room_id = -1;

	std::array<std::shared_ptr<Session>, 4> sessions;	// �� �濡�� 4���� �ִ�

	std::unordered_map<int, std::shared_ptr<MonsterObject>> monsters;

	std::mutex room_mutex;

public:
	Room(int room_id);
	void addSession(std::shared_ptr<Session> session);
	void update(float elapsedTime);

};

