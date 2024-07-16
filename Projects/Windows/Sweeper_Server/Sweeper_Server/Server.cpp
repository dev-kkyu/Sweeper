#include "Server.h"

#include "Session.h"

#include <iostream>
#include <cmath>

// ���� �б�� ���
#include <fstream>
#include <string>
#include <sstream>

#define MONSTER_FILE_NAME "resources/MonsterData.txt"

Server::Server(asio::io_context& io_context, int port)
	: io_context{ io_context }
	, acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
	, timer{ io_context, std::chrono::steady_clock::now() }		// ���� �ð��� ����� ���ش�.
	, last_time{ std::chrono::steady_clock::now() }
{
	// ���Ͽ��� ���� ���� �о����
	loadMonsterInfo();

	// �� ����
	// Todo: ���� ������ Ȯ�� �ʿ�
	room = std::make_shared<Room>(io_context, 0, initMonsterInfo);			// �ϴ� ���ȣ 0�� ����

	fps_value = 60;			// �ʴ� 60�� ������Ʈ

	std::cout << "���� �غ� �Ϸ�" << std::endl;

	doAccept();				// ������ �ޱ� �����Ѵ�.
	doTimer();				// Ÿ�̸� ����(������Ʈ)�� �����Ѵ�.
}

void Server::loadMonsterInfo()
{
	std::ifstream in{ MONSTER_FILE_NAME };
	if (!in) {
		std::cerr << "failed to open monster info file!" << std::endl;
		exit(-1);
	}
	std::string line;
	while (std::getline(in, line)) {
		std::stringstream ss{ line };
		char ignore;
		int mosterType;
		float posx, posy, posz, rotx, roty, rotz;
		ss >> ignore >> mosterType >> ignore;
		ss >> ignore >> posx >> ignore >> posy >> ignore >> posz >> ignore;
		ss >> ignore >> rotx >> ignore >> roty >> ignore >> rotz >> ignore;
		MonsterInfo info{ static_cast<MONSTER_TYPE>(mosterType), posx, posy, posz, roty };
		initMonsterInfo.push_back(info);
	}
	std::cout << "���� �ʱ�ȭ ���� �ε� �Ϸ�" << std::endl;
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// ���������� ������ �޾� �۾��� �Ϸ����� ��

				asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
				std::string remote_ip = remote_endpoint.address().to_string();
				asio::ip::port_type remote_port = remote_endpoint.port();

				std::cout << "INFO:ACCEPT [" << remote_ip << ":" << remote_port << "] ����" << std::endl;

				auto ptr = std::make_shared<Session>(std::move(socket));

				// ������ �濡 �߰����ش�. ������ �÷��̾ ����. Todo : ������ ���� ã�Ƽ� �־���� �Ѵ�.
				room->addSession(ptr);

				doAccept();	// ���� ������ ��ٸ���.
			}
		});
}

void Server::doTimer()
{
	timer.expires_at(timer.expiry() + std::chrono::nanoseconds(static_cast<int>(1. / fps_value * 1'000'000'000.)));		// ������ fps�� ���缭 ���� �ð� ����
	timer.async_wait(
		[this](asio::error_code ec)
		{
			if (!ec) {
				auto now_time = std::chrono::steady_clock::now();
				auto elapsed_time = now_time - last_time;
				last_time = now_time;
#ifndef NDEBUG															// ����� ����� ��, 1�ʿ� �ѹ��� ���� fps�� ������ش�.
				static std::chrono::nanoseconds accm_time;
				static int frame;
				static int fps;
				++frame;
				accm_time += elapsed_time;
				if (accm_time >= std::chrono::seconds{ 1 }) {			// �ʴ� �ѹ��� FPS ǥ��
					float over_time = accm_time.count() / 1'000'000'000.f;
					fps = static_cast<int>(std::round(frame * (1.f / over_time)));
					frame -= fps;
					accm_time -= std::chrono::seconds{ 1 };

					std::cout << fps << "fps" << std::endl;
				}
#endif
				update(elapsed_time.count() / 1'000'000'000.f);			// ���� ������Ʈ

				doTimer();												// Ÿ�̸� ��ȣ��
			}
			else {
				std::cerr << "Server Timer Error" << std::endl;
			}
		});
}

void Server::update(float elapsedTime)
{
	room->update(elapsedTime);
}
