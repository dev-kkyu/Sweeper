#include <iostream>
#include <vector>
#include <thread>

#include "Server.h"

void workerThread(asio::io_context* context)
{
	context->run();
}

int main()
{
	asio::io_context io_context;
	Server server{ io_context, SERVER_PORT };			// 서버 열기

	std::vector<std::thread> worker_threads;

	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(workerThread, &io_context);

	for (auto& th : worker_threads)
		th.join();
}
