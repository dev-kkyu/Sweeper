#include "NetworkManager.h"

#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>

// tcp::socket Ŭ������ ���漱���� �Ұ��Ͽ�, void*�� ��ü�ϴ� ��� ����� �����Լ�
static asio::ip::tcp::socket& getSocket(void* p_asio_tcp_socket)
{
	return *static_cast<asio::ip::tcp::socket*>(p_asio_tcp_socket);
}

NetworkManager::NetworkManager()
{
	p_io_context = std::make_shared<asio::io_context>();
	p_socket = new asio::ip::tcp::socket{ *p_io_context };

	remain_size = 0;
	my_id = -1;
}

NetworkManager::~NetworkManager()
{
	delete (&getSocket(p_socket));
	p_io_context.reset();
}

NetworkManager& NetworkManager::getInstance()
{
	static NetworkManager instance;		// C++11 ���� Thread-Safe �ϴ�.
	return instance;
}

void NetworkManager::connectServer(std::string ipAddress)
{
	asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(ipAddress), SERVER_PORT);
	asio::error_code ec;
	getSocket(p_socket).connect(endpoint, ec);		// connect�� �񵿱�� �� �ʿ� ����δ�. ������ ���� �ȵǸ� �ٷ� ���ϵ�.

	if (!ec) {		// connect ������
		std::cout << "Connect To : " << ipAddress << ":" << SERVER_PORT << " Succeeded." << std::endl;
		// Todo : ...
	}
	else {			// ���н�
		std::cerr << "Connect Error[" << ec << "]: " << ec.message() << std::endl;
		exit(-1);
	}
}

void NetworkManager::start(PLAYER_TYPE player_type)
{
	// �α��� ��Ŷ ����
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;
	p.player_type = player_type;
	sendPacket(&p);

	doRead();		// �����ϱ⸦ �����Ѵ�.

}

void NetworkManager::poll()
{
	p_io_context->poll();
}

void NetworkManager::stop()
{
	p_io_context->stop();
	getSocket(p_socket).close();
}

void NetworkManager::sendPacket(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];				// doWrite�� ���� �Լ����� ������ �� ���̴�.
	memcpy(buff, packet, packet_size);
	doWrite(buff, packet_size);
}

void NetworkManager::setPacketReceivedCallback(std::function<void(unsigned char*)> callback)
{
	processPacketFunc = callback;
}

void NetworkManager::doRead()
{
	getSocket(p_socket).async_read_some(asio::buffer(read_buffer),
		[this](asio::error_code ec, std::size_t length)
		{
			if (ec) {		// �������� ��
				if (ec.value() == asio::error::operation_aborted)
					return;
				std::cout << "Receive Error on Session[" << my_id << "]: [" << ec << "]: " << ec.message() << std::endl;
				std::cout << "��Ʈ��ũ ������ ����Ǿ����ϴ�" << std::endl;
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

void NetworkManager::doWrite(unsigned char* packet, std::size_t length)
{
	// �Լ��� ù��° ���ڷ� ���� buffer�� �񵿱� �۾� ���� �޸𸮰� �Ҹ���� �ʵ��� �޸𸮸� ���α׷��� ������ ��� �Ѵ�.
	getSocket(p_socket).async_write_some(asio::buffer(packet, length),
		[this, packet, length](asio::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)		// send �Ϸ� ��
			{
				if (length != bytes_transferred) {
					std::cout << "Incomplete Send occured on session[" << my_id << "]. This session should be closed.\n";
				}
				delete[] packet;		// �۾��� �Ϸ�Ǿ����Ƿ� ���̻� �ʿ����, ���θ� Leak�̴�.	// �Ҹ��ڰ� ���� ��ü�� delete, delete[] �������
			}
		});
}

void NetworkManager::processPacket(unsigned char* packet)
{
	// ��Ŷ ó�� �ݹ� �Լ��� �����Ǿ� ������ ȣ��
	if (processPacketFunc) {
		processPacketFunc(packet);
	}
}
