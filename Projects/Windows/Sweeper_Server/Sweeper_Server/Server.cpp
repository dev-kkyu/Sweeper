#include "Server.h"

#include "Room.h"
#include "Session.h"

#include <iostream>
#include <cmath>

Server::Server(asio::io_context& io_context, int port)
	: acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
	, timer{ io_context, std::chrono::steady_clock::now() }		// 만료 시간을 현재로 해준다.
	, last_time{ std::chrono::steady_clock::now() }
{
	// 방 생성
	// Todo: 여러 방으로 확장 필요
	room = std::make_shared<Room>(0);			// 일단 방번호 0번 설정

	fps_value = 60;			// 초당 60번 업데이트

	doAccept();				// 접속을 받기 시작한다.
	doTimer();				// 타이머 동작(업데이트)을 시작한다.
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
	// Todo: 업데이트 로직 추가
}
