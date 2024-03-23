#pragma once

#define SERVER_PORT		7777
#define BUFF_SIZE		1024
#define MAX_ROOM		500

constexpr char SC_LOGIN = 1;
constexpr char SC_LOGOUT = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_POSITION = 4;
constexpr char SC_PLAYER_LOOK = 5;

constexpr char CS_KEY_EVENT = 1;
constexpr char CS_MOVE_MOUSE = 2;

enum class MY_KEY_EVENT : char
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
	SPACE = 4
};

#pragma pack(push, 1)

struct SC_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char room_id;
	char player_id;
};

struct SC_LOGOUT_PACKET
{
	unsigned char size;
	char type;
	char player_id;
};

struct SC_ADD_PLAYER_PACKET
{
	unsigned char size;
	char type;
	char player_id;
};

struct SC_POSITION_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	float x;
	float y;
	float z;
};

struct SC_PLAYER_LOOK_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	float dir_x;
	float dir_z;
};

struct CS_KEY_EVENT_PACKET
{
	unsigned char size;
	char type;
	bool is_pressed;
	MY_KEY_EVENT key;
};

struct CS_MOVE_MOUSE_PACKET
{
	unsigned char size;
	char type;
	float move_x;
	float move_y;
};

#pragma pack(pop)

