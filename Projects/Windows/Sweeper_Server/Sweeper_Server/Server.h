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

	// 서버 업데이트 관련 변수
	asio::steady_timer timer;
	std::chrono::steady_clock::time_point last_time;
	int fps_value;

	// 방, 추후 여러개로 확장해야 함
	std::shared_ptr<Room> room;

public:
	Server(asio::io_context& io_context, int port);

private:
	void doAccept();
	void doTimer();

	void update(float elapsedTime);

};

