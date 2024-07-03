#pragma once

#include <list>
#include <memory>
#include <chrono>

#define ASIO_STANDALONE
#include <asio.hpp>

#include "Room.h"

class Server
{
private:
	asio::io_context& io_context;
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	// ���� ������Ʈ ���� ����
	asio::steady_timer timer;
	std::chrono::steady_clock::time_point last_time;
	int fps_value;

	// �ʱ� ���� ����Ʈ	// ���� ������ ������ �о����, ������ �Ѱ��ش�.
	std::list<MonsterInfo> initMonsterInfo;

	// ��, ���� �������� Ȯ���ؾ� ��
	std::shared_ptr<Room> room;

public:
	Server(asio::io_context& io_context, int port);

private:
	void loadMonsterInfo();

	void doAccept();
	void doTimer();

	void update(float elapsedTime);

};

