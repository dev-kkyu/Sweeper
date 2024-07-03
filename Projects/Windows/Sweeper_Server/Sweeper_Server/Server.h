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

	// 서버 업데이트 관련 변수
	asio::steady_timer timer;
	std::chrono::steady_clock::time_point last_time;
	int fps_value;

	// 초기 몬스터 리스트	// 서버 생성시 파일을 읽어오고, 방으로 넘겨준다.
	std::list<MonsterInfo> initMonsterInfo;

	// 방, 추후 여러개로 확장해야 함
	std::shared_ptr<Room> room;

public:
	Server(asio::io_context& io_context, int port);

private:
	void loadMonsterInfo();

	void doAccept();
	void doTimer();

	void update(float elapsedTime);

};

