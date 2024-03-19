#pragma once

#include <memory>

#define ASIO_STANDALONE
#include "includes/asio.hpp"

class Room;
class Server
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	std::shared_ptr<Room> room;

public:
	Server(asio::io_context& io_context, int port);

private:
	void doAccept();

};

