#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#define ASIO_STANDALONE
#include "asio.hpp"

#define SERVER_PORT		7777
#define BUFF_SIZE		1024
#define MAX_USER		2000

class Session;
std::array<std::shared_ptr<Session>, MAX_USER> players;
std::mutex g_mutex;

int getNewClientID()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (!players[i])
			return i;
	}
	std::cout << "MAX USER FULL\n";
	exit(-1);
	return -1;
}

class Session
	: public std::enable_shared_from_this<Session>		// �񵿱� �ݹ�(�ַ� �����Լ�) ȣ��ñ��� ��ü�� �����ֱ� ���Ͽ�(����Ʈ������) ��������� ��
{
private:
	asio::ip::tcp::socket socket;
	int my_id;

	// recv�� ����
	unsigned char read_buffer[BUFF_SIZE];
	unsigned char remain_data[BUFF_SIZE];
	int remain_size;

public:
	Session(asio::ip::tcp::socket socket, int new_id) : socket{ std::move(socket) }, my_id{ new_id }
	{
		remain_size = 0;
	}

	void start()
	{
		doRead();		// �����ϱ⸦ �����Ѵ�.

		// Todo : ���� ���ӽ� �� ��
	}

	void sendPacket(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];				// doWrite�� ���� �Լ����� ������ �� ���̴�.
		memcpy(buff, packet, packet_size);
		doWrite(buff, packet_size);
	}

private:
	void doRead()
	{
		auto self(shared_from_this());
		socket.async_read_some(asio::buffer(read_buffer),
			[this, self](asio::error_code ec, std::size_t length)
			{
				if (ec) {		// �������� ��
					if (ec.value() == asio::error::operation_aborted)
						return;
					std::cout << "Receive Error on Session[" << my_id << "]: [" << ec << "]: " << ec.message() << std::endl;
					players[my_id] = nullptr;													// ���� �����Ѵ�.
					return;
				}

				int received = static_cast<int>(length);
				unsigned char* ptr = read_buffer;

				// ��Ŷ ������
				while (received > 0) {
					int packet_size = ptr[0];
					if (remain_size > 0) {
						packet_size = remain_data[0];
					}

					if (remain_size + received >= packet_size) {							// ������ �ִ� ������ ũ�Ⱑ ó���� ������ ũ�� �̻��϶�
						memcpy(remain_data + remain_size, ptr, packet_size - remain_size);	// ó���� �����͸�ŭ ��Ŷ�� ������ش�.
						ptr += packet_size - remain_size;									// ������ ó���� �������� ���� ��ġ�� �̸� �̵�
						received -= packet_size - remain_size;								// ������ ���� ������ ũ�⸦ ����
						processPacket(remain_data);											// ���� ��Ŷ�� ���� ó���ϱ�
						remain_size = 0;													// remain_size�� received�� ���յǾ��� ������ 0���� ������ش�.
					}
					else {
						memcpy(remain_data + remain_size, ptr, received);					// ������ ó���� ������ �����صα�
						remain_size += received;
						received = 0;
					}
				}

				doRead();	// ���� ���� �����͸� ��ٸ���.
			});
	}

	void doWrite(unsigned char* packet, std::size_t length)
	{
		auto self(shared_from_this());
		// �Լ��� ù��° ���ڷ� ���� buffer�� �񵿱� �۾� ���� �޸𸮰� �Ҹ���� �ʵ��� �޸𸮸� ���α׷��� ������ ��� �Ѵ�.
		socket.async_write_some(asio::buffer(packet, length),
			[this, self, packet, length](asio::error_code ec, std::size_t bytes_transferred)
			{
				// shared_ptr�� self�� �� �ݹ��� ȣ�� �� ���� �� ������ �����Ѵ�. -> this�� �����ϰ� �ֱ� ������ �ʿ��ϴ�. -> use_count �÷��ش�.
				// ���� �Լ����� ������ ĸó�� �߱� �����̴�. -> ���� ���� ��ü�� �����ϴ� ���� ���, ���� �ֱⰡ ���� �ݹ��� ������ ����ȴ�.
				if (!ec)		// send �Ϸ� ��
				{
					if (length != bytes_transferred) {
						std::cout << "Incomplete Send occured on session[" << my_id << "]. This session should be closed.\n";
					}
					delete[] packet;		// �۾��� �Ϸ�Ǿ����Ƿ� ���̻� �ʿ����, ���θ� Leak�̴�.	// �Ҹ��ڰ� ���� ��ü�� delete, delete[] �������
				}
			});
	}

	void processPacket(unsigned char* packet)
	{
		// Todo : ���� ��Ŷ ó��

		// �׽�Ʈ�� ���� �׻� str�̶�� ������ ó��
		int packet_size = packet[0];
		std::string str{ reinterpret_cast<char*>(packet + 1), static_cast<size_t>(packet_size - 1) };
		std::cout << "Recv str: " << str << std::endl;
	}
};

class Server
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

public:
	Server(asio::io_context& io_context, int port)
		: acceptor{ io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }, socket{ io_context }
	{
		do_accept();				// ������ �ޱ� �����Ѵ�.
	}

private:
	void do_accept()
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

					do_accept();	// ���� ������ ��ٸ���.
				}
			});
	}
};

void workerThread(asio::io_context* context)
{
	context->run();
}

int main()
{
	asio::io_context io_context;
	Server server{ io_context, SERVER_PORT };			// ���� ����

	std::vector<std::thread> worker_threads;

	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(workerThread, &io_context);

	for (auto& th : worker_threads)
		th.join();
}
