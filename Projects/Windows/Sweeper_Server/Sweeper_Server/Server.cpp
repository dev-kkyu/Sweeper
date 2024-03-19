#include "Server.h"

#include "Room.h"
#include "Session.h"

#include <iostream>

Server::Server(asio::io_context& io_context, int port)
	: acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
{
	// �� ����. ���� ���� ������ Ȯ�� �ʿ�
	room = std::make_shared<Room>(0);			// �ϴ� ���ȣ 0�� ����

	doAccept();				// ������ �ޱ� �����Ѵ�.
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// ���������� ������ �޾� �۾��� �Ϸ����� ��

				auto ptr = std::make_shared<Session>(std::move(socket));

				room->addPlayer(ptr);

				doAccept();	// ���� ������ ��ٸ���.
			}
		});
}
