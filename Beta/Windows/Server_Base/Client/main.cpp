#include <iostream>
#include <vector>

#define ASIO_STANDALONE
#include "asio.hpp"

#define SERVER_PORT		7777
#define BUFF_SIZE		1024

asio::io_context io_context;

class Session
	: public std::enable_shared_from_this<Session>		// 비동기 콜백(주로 람다함수) 호출시까지 객체가 남아있기 위하여(스마트포인터) 설정해줘야 함
{
private:
	asio::ip::tcp::socket socket;
	int my_id;

	// recv용 변수
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
		doRead();		// 수신하기를 시작한다.

		// Todo : 최초 접속시 할 일
	}

	void sendPacket(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];				// doWrite의 람다 함수에서 해제해 줄 것이다.
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
				if (ec) {		// 실패했을 때
					if (ec.value() == asio::error::operation_aborted)
						return;
					std::cout << "Receive Error on Session[" << my_id << "]: [" << ec << "]: " << ec.message() << std::endl;
					return;
				}

				int received = static_cast<int>(length);
				unsigned char* ptr = read_buffer;

				// 패킷 재조립
				while (received > 0) {
					int packet_size = ptr[0];
					if (remain_size > 0) {
						packet_size = remain_data[0];
					}

					if (remain_size + received >= packet_size) {							// 가지고 있는 데이터 크기가 처리할 데이터 크기 이상일때
						memcpy(remain_data + remain_size, ptr, packet_size - remain_size);	// 처리할 데이터만큼 패킷을 만들어준다.
						ptr += packet_size - remain_size;									// 다음에 처리할 데이터의 시작 위치로 미리 이동
						received -= packet_size - remain_size;								// 다음에 남은 데이터 크기를 저장
						processPacket(remain_data);											// 만든 패킷에 대한 처리하기
						remain_size = 0;													// remain_size는 received로 통합되었기 때문에 0으로 만들어준다.
					}
					else {
						memcpy(remain_data + remain_size, ptr, received);					// 다음에 처리할 데이터 저장해두기
						remain_size += received;
						received = 0;
					}
				}

				// Client는 run_for 1us를 해서 콜백함수를 호출한다.
				io_context.run_for(std::chrono::microseconds(1));

				doRead();	// 다음 수신 데이터를 기다린다.
			});
	}

	void doWrite(unsigned char* packet, std::size_t length)
	{
		auto self(shared_from_this());
		// 함수의 첫번째 인자로 들어가는 buffer는 비동기 작업 동안 메모리가 소멸되지 않도록 메모리를 프로그램이 관리해 줘야 한다.
		socket.async_write_some(asio::buffer(packet, length),
			[this, self, packet, length](asio::error_code ec, std::size_t bytes_transferred)
			{
				// shared_ptr인 self는 이 콜백이 호출 및 리턴 될 때까지 존재한다. -> this를 참조하고 있기 때문에 필요하다. -> use_count 올려준다.
				// 람다 함수에서 값으로 캡처를 했기 때문이다. -> 만약 현재 객체를 참조하는 곳이 없어도, 생명 주기가 현재 콜백의 끝까지 연장된다.
				if (!ec)		// send 완료 시
				{
					if (length != bytes_transferred) {
						std::cout << "Incomplete Send occured on session[" << my_id << "]. This session should be closed.\n";
					}
					delete[] packet;		// 작업이 완료되었으므로 더이상 필요없고, 놔두면 Leak이다.	// 소멸자가 없는 객체는 delete, delete[] 상관없음

					// Client는 run_for 1us를 해서 콜백함수를 호출한다.
					io_context.run_for(std::chrono::microseconds(1));
				}
			});
	}

	void processPacket(unsigned char* packet)
	{
		// Todo : 수신 패킷 처리
	}
};

int main()
{
	asio::ip::tcp::socket socket{ io_context };
	asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), SERVER_PORT);
	asio::error_code ec;
	socket.connect(endpoint, ec);		// connect는 비동기로 할 필요 없어보인다. 어차피 연결 안되면 바로 리턴됨.

	std::shared_ptr<Session> session;
	if (!ec) {
		session = std::make_shared<Session>(std::move(socket), 0);
		session->start();
	}
	else {
		std::cerr << "Connect Error[" << ec << "]: " << ec.message() << std::endl;
		exit(-1);
	}

	// Todo : session을 shared_ptr로 할지, 지역/전역 변수로 할지 고민 필요

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

	// Todo : io_context는 외부에 두고, 게임루프에서 단한번 호출 -> run_for(microsecond(1)) 하기.
	// 성공해서 콜백함수가 호출되면, 콜백함수 마지막에서 run_for(microsecond(1)) 호출 (nanosecond 테스트 해보니 안됨)
	// -> 루프마다, 1us의 딜레이를 주어 완료된 작업이 있으면 꺼내서 콜백함수 호출해 주는 효과.
	// 1us는 0.000001초이므로, 유의미한 지연을 발생시키지 않는다. (게임에서 10000프레임이 나온다고 가정해도, 프레임당 0.0001초이다.)

	io_context.run();

}
