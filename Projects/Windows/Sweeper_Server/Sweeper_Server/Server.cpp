#include "Server.h"
#include <iostream>
#include <array>
#include <mutex>

// ���� ����
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
	doAccept();				// ������ �ޱ� �����Ѵ�.
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// ���������� ������ �޾� �۾��� �Ϸ����� ��
				g_mutex.lock();
				int p_id = getNewClientID();
				players[p_id] = std::make_shared<Session>(std::move(socket), p_id);
				g_mutex.unlock();
				players[p_id]->start();

				std::cout << "�÷��̾� " << p_id << " ����\n";

				doAccept();	// ���� ������ ��ٸ���.
			}
		});
}
