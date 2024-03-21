#pragma once

#include <memory>
#include <chrono>

#define ASIO_STANDALONE
#include "includes/asio.hpp"

class Room;
class Server
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	// ���� ������Ʈ ���� ����
	asio::steady_timer timer;
	std::chrono::steady_clock::time_point last_time;
	int fps_value;

	// ��, ���� �������� Ȯ���ؾ� ��
	std::shared_ptr<Room> room;

public:
	Server(asio::io_context& io_context, int port);

private:
	void doAccept();
	void doTimer();

	void update(float elapsedTime);

};

