#include "Server.h"
#include <iostream>
#include <array>
#include <mutex>

// 전역 변수
std::array<std::shared_ptr<Session>, MAX_USER> players;
std::mutex g_mutex;

static int getNewClientID()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (!players[i])
			return i;
	}
	std::cout << "MAX USER FULL\n";
	exit(-1);
	return -1;
}

Server::Server(asio::io_context& io_context, int port)
	: acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
{
	doAccept();				// 접속을 받기 시작한다.
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// 정상적으로 접속을 받아 작업을 완료했을 때
				g_mutex.lock();
				int p_id = getNewClientID();
				players[p_id] = std::make_shared<Session>(std::move(socket), p_id);
				g_mutex.unlock();
				players[p_id]->start();

				std::cout << "플레이어 " << p_id << " 접속\n";

				doAccept();	// 다음 접속을 기다린다.
			}
		});
}
