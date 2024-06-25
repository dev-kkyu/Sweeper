#pragma once

#define SERVER_PORT		7777
#define BUFF_SIZE		1024
#define MAX_ROOM		500

constexpr char SC_LOGIN_INFO = 1;
constexpr char SC_LOGOUT = 2;
constexpr char SC_LOGIN_FAIL = 3;
constexpr char SC_ADD_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;
constexpr char SC_PLAYER_LOOK = 6;
constexpr char SC_PLAYER_STATE = 7;
constexpr char SC_CLIENT_KEY_EVENT = 8;
constexpr char SC_ADD_MONSTER = 9;
constexpr char SC_MOVE_MONSTER = 10;
constexpr char SC_MONSTER_LOOK = 11;
constexpr char SC_REMOVE_MONSTER = 12;
constexpr char SC_MONSTER_STATE = 13;

constexpr char CS_LOGIN = 1;
constexpr char CS_KEY_EVENT = 2;

constexpr float PLAYER_SPEED = 5.f;

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_LEFT		0x04
#define KEY_RIGHT		0x08
#define KEY_SPACE		0x10	// ��� SPACE�� ������ �����ȴ�.
#define KEY_MOUSE_RIGHT	0x20

enum class PLAYER_TYPE : char
{
	WARRIOR = 0,
	ARCHER = 1,
	MAGE = 2,
	HEALER = 3,
};

enum class PLAYER_STATE : char
{
	IDLE = 0,
	RUN = 1,
	ATTACK = 2
};

enum class MONSTER_STATE : char
{
	IDLE = 0,
	MOVE = 1,
	HIT = 2,
	DIE = 3,
	ATTACK = 4
};

#pragma pack(push, 1)

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	char room_id;
	char player_id;
	float pos_x;		// ���� �÷��̾� ��ġ
	float pos_z;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_LOGOUT_PACKET
{
	unsigned char size;
	char type;
	char player_id;
};

struct SC_LOGIN_FAIL_PACKET			// �ϴ��� Fail ������ �� ������ �� �ϳ���, ���� �ʿ�� Ȯ��
{
	unsigned char size;
	char type;
};

struct SC_ADD_PLAYER_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	PLAYER_TYPE player_type;
	float pos_x;		// ���� �÷��̾� ��ġ
	float pos_z;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_MOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	float pos_x;
	float pos_y;
	float pos_z;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_PLAYER_LOOK_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	float dir_x;
	float dir_z;
};

struct SC_PLAYER_STATE_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	PLAYER_STATE state;
};

struct SC_CLIENT_KEY_EVENT_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	bool is_pressed;
	unsigned char key;
};

struct SC_ADD_MONSTER_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	float pos_x;		// ���� ��ġ
	float pos_z;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_MOVE_MONSTER_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	float pos_x;		// ���� ��ġ
	float pos_z;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_MONSTER_LOOK_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	float dir_x;		// ���� �����ִ� ����
	float dir_z;
};

struct SC_REMOVE_MONSTER_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
};

struct SC_MONSTER_STATE_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	MONSTER_STATE state;
};

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	PLAYER_TYPE player_type;
};

struct CS_KEY_EVENT_PACKET
{
	unsigned char size;
	char type;
	bool is_pressed;
	unsigned char key;
};

#pragma pack(pop)

