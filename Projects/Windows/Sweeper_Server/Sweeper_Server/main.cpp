#include <iostream>
#include <vector>
#include <thread>

#include "Server.h"
#include "protocol.h"

#include "Map.h"

void workerThread(asio::io_context* context)
{
	context->run();
}

int main()
{
	std::cout << "맵 로드 중..." << std::endl;
	Map::getInstance();		// 최초 한번 호출로, 싱글톤 객체 생성 (모델 로드 미리 해야 함)
	std::cout << "맵 로드 완료" << std::endl;

	asio::io_context io_context;
	Server server{ io_context, SERVER_PORT };			// 서버 열기

	std::vector<std::thread> worker_threads;

	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(workerThread, &io_context);

	for (auto& th : worker_threads)
		th.join();
}
