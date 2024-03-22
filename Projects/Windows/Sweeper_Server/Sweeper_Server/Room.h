#pragma once

#include <array>
#include <memory>
#include <mutex>

class Session;
class Room
{
private:
public:
	int room_id = -1;

	std::array<std::shared_ptr<Session>, 4> sessions;	// �� �濡�� 4���� �ִ�

	std::mutex room_mutex;

public:
	Room(int room_id);
	void addSession(std::shared_ptr<Session> session);
	void update(float elapsedTime);

};

