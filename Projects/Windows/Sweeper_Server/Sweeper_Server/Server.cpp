#include "Server.h"

#include "Room.h"
#include "Session.h"

#include <iostream>

Server::Server(asio::io_context& io_context, int port)
	: acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
{
	// 방 생성. 추후 여러 방으로 확장 필요
	room = std::make_shared<Room>(0);			// 일단 방번호 0번 설정

	doAccept();				// 접속을 받기 시작한다.
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// 정상적으로 접속을 받아 작업을 완료했을 때

				auto ptr = std::make_shared<Session>(std::move(socket));

				room->addPlayer(ptr);

				doAccept();	// 다음 접속을 기다린다.
			}
		});
}
