#pragma once

#include "Session.h"

class Server
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

public:
	Server(asio::io_context& io_context, int port);

private:
	void do_accept();

};

