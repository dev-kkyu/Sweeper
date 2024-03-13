#include <iostream>
#include <vector>

#define ASIO_STANDALONE
#include "asio.hpp"

#define SERVER_PORT		7777
#define BUFF_SIZE		1024

asio::io_context io_context;

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

				// Client�� run_for 1us�� �ؼ� �ݹ��Լ��� ȣ���Ѵ�.
				io_context.run_for(std::chrono::microseconds(1));

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

					// Client�� run_for 1us�� �ؼ� �ݹ��Լ��� ȣ���Ѵ�.
					io_context.run_for(std::chrono::microseconds(1));
				}
			});
	}

	void processPacket(unsigned char* packet)
	{
		// Todo : ���� ��Ŷ ó��
	}
};

int main()
{
	asio::ip::tcp::socket socket{ io_context };
	asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), SERVER_PORT);
	asio::error_code ec;
	socket.connect(endpoint, ec);		// connect�� �񵿱�� �� �ʿ� ����δ�. ������ ���� �ȵǸ� �ٷ� ���ϵ�.

	std::shared_ptr<Session> session;
	if (!ec) {
		session = std::make_shared<Session>(std::move(socket), 0);
		session->start();
	}
	else {
		std::cerr << "Connect Error[" << ec << "]: " << ec.message() << std::endl;
		exit(-1);
	}

	// Todo : session�� shared_ptr�� ����, ����/���� ������ ���� ��� �ʿ�

	while (true) {
		std::string str;
		std::cout << "Send str: ";
		std::cin >> str;

		std::vector<unsigned char> data(str.size() + 1);
		data[0] = static_cast<unsigned char>(data.size());
		memcpy(data.data() + 1, str.c_str(), str.size());
		session->sendPacket(data.data());
		io_context.run_for(std::chrono::microseconds(1));
	}

	// Todo : io_context�� �ܺο� �ΰ�, ���ӷ������� ���ѹ� ȣ�� -> run_for(microsecond(1)) �ϱ�.
	// �����ؼ� �ݹ��Լ��� ȣ��Ǹ�, �ݹ��Լ� ���������� run_for(microsecond(1)) ȣ�� (nanosecond �׽�Ʈ �غ��� �ȵ�)
	// -> ��������, 1us�� �����̸� �־� �Ϸ�� �۾��� ������ ������ �ݹ��Լ� ȣ���� �ִ� ȿ��.
	// 1us�� 0.000001���̹Ƿ�, ���ǹ��� ������ �߻���Ű�� �ʴ´�. (���ӿ��� 10000�������� ���´ٰ� �����ص�, �����Ӵ� 0.0001���̴�.)

	io_context.run();

}
