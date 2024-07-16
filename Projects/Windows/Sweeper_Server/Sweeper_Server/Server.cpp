#include "Server.h"

#include "Session.h"

#include <iostream>
#include <cmath>

// 파일 읽기용 헤더
#include <fstream>
#include <string>
#include <sstream>

#define MONSTER_FILE_NAME "resources/MonsterData.txt"

Server::Server(asio::io_context& io_context, int port)
	: io_context{ io_context }
	, acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
	, timer{ io_context, std::chrono::steady_clock::now() }		// 만료 시간을 현재로 해준다.
	, last_time{ std::chrono::steady_clock::now() }
{
	// 파일에서 몬스터 정보 읽어오기
	loadMonsterInfo();

	// 방 생성
	// Todo: 여러 방으로 확장 필요
	room = std::make_shared<Room>(io_context, 0, initMonsterInfo);			// 일단 방번호 0번 설정

	fps_value = 60;			// 초당 60번 업데이트

	std::cout << "서버 준비 완료" << std::endl;

	doAccept();				// 접속을 받기 시작한다.
	doTimer();				// 타이머 동작(업데이트)을 시작한다.
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
	std::cout << "몬스터 초기화 파일 로드 완료" << std::endl;
}

void Server::doAccept()
{
	acceptor.async_accept(socket,
		[this](asio::error_code ec)
		{
			if (!ec) {		// 정상적으로 접속을 받아 작업을 완료했을 때

				asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
				std::string remote_ip = remote_endpoint.address().to_string();
				asio::ip::port_type remote_port = remote_endpoint.port();

				std::cout << "INFO:ACCEPT [" << remote_ip << ":" << remote_port << "] 접속" << std::endl;

				auto ptr = std::make_shared<Session>(std::move(socket));

				// 세션을 방에 추가해준다. 세션은 플레이어를 가짐. Todo : 적절한 방을 찾아서 넣어줘야 한다.
				room->addSession(ptr);

				doAccept();	// 다음 접속을 기다린다.
			}
		});
}

void Server::doTimer()
{
	timer.expires_at(timer.expiry() + std::chrono::nanoseconds(static_cast<int>(1. / fps_value * 1'000'000'000.)));		// 지정한 fps에 맞춰서 만료 시간 설정
	timer.async_wait(
		[this](asio::error_code ec)
		{
			if (!ec) {
				auto now_time = std::chrono::steady_clock::now();
				auto elapsed_time = now_time - last_time;
				last_time = now_time;
#ifndef NDEBUG															// 디버그 모드일 땐, 1초에 한번씩 현재 fps를 출력해준다.
				static std::chrono::nanoseconds accm_time;
				static int frame;
				static int fps;
				++frame;
				accm_time += elapsed_time;
				if (accm_time >= std::chrono::seconds{ 1 }) {			// 초당 한번씩 FPS 표시
					float over_time = accm_time.count() / 1'000'000'000.f;
					fps = static_cast<int>(std::round(frame * (1.f / over_time)));
					frame -= fps;
					accm_time -= std::chrono::seconds{ 1 };

					std::cout << fps << "fps" << std::endl;
				}
#endif
				update(elapsed_time.count() / 1'000'000'000.f);			// 서버 업데이트

				doTimer();												// 타이머 재호출
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
