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
constexpr char SC_PLAYER_HP = 8;
constexpr char SC_ADD_ARROW = 9;
constexpr char SC_MOVE_ARROW = 10;
constexpr char SC_REMOVE_ARROW = 11;
constexpr char SC_CLIENT_KEY_EVENT = 12;
constexpr char SC_ADD_MONSTER = 13;
constexpr char SC_MOVE_MONSTER = 14;
constexpr char SC_MONSTER_LOOK = 15;
constexpr char SC_REMOVE_MONSTER = 16;
constexpr char SC_MONSTER_STATE = 17;
constexpr char SC_MONSTER_HP = 18;
constexpr char SC_MOVE_BOSS = 19;
constexpr char SC_BOSS_STATE = 20;
constexpr char SC_BOSS_HP = 21;

constexpr char CS_LOGIN = 1;
constexpr char CS_KEY_EVENT = 2;

constexpr float PLAYER_SPEED = 5.f;

constexpr short MAX_HP_PLAYER_WARRIOR = 1000;
constexpr short MAX_HP_PLAYER_ARCHER = 1000;
constexpr short MAX_HP_PLAYER_MAGE = 1000;
constexpr short MAX_HP_PLAYER_HEALER = 1000;
constexpr short MAX_HP_MONSTER_MUSHROOM = 300;
constexpr short MAX_HP_MONSTER_BORNDOG = 300;
constexpr short MAX_HP_MONSTER_GOBLIN = 300;
constexpr short MAX_HP_MONSTER_BOOGIE = 300;
constexpr short MAX_HP_BOSS = 10000;

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_LEFT		0x04
#define KEY_RIGHT		0x08
#define KEY_SPACE		0x10
#define KEY_CTRL		0x20
#define MOUSE_LEFT		0x40
#define MOUSE_RIGHT		0x80

enum class PLAYER_TYPE : char
{
	WARRIOR = 0,
	ARCHER = 1,
	MAGE = 2,
	HEALER = 3
};

enum class PLAYER_STATE : char
{
	IDLE = 0,
	RUN = 1,
	DASH = 2,
	ATTACK = 3,
	SKILL = 4
};

enum class MONSTER_TYPE : char
{
	MUSHROOM = 0,
	BORNDOG = 1,
	GOBLIN = 2,
	BOOGIE = 3
};

enum class MONSTER_STATE : char
{
	IDLE = 0,
	MOVE = 1,
	HIT = 2,
	DIE = 3,
	ATTACK = 4
};

enum class BOSS_STATE : char
{
	SLEEP = 0,
	WAKEUP = 1,
	IDLE = 2,
	MOVE = 3,
	LEFT_PUNCH = 4,
	RIGHT_PUNCH = 5,
	PUNCH_DOWN = 6,
	DIE = 7
};

#pragma pack(push, 1)

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	char room_id;
	char player_id;
	float pos_x;		// 현재 플레이어 위치
	float pos_z;
	float dir_x;		// 현재 보고있는 방향
	float dir_z;
};

struct SC_LOGOUT_PACKET
{
	unsigned char size;
	char type;
	char player_id;
};

struct SC_LOGIN_FAIL_PACKET			// 일단은 Fail 사유가 방 가득찬 것 하나뿐, 추후 필요시 확장
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
	short hp;			// 플레이어의 현재 HP
	float pos_x;		// 현재 플레이어 위치
	float pos_z;
	float dir_x;		// 현재 보고있는 방향
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
	float dir_x;		// 현재 보고있는 방향
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

struct SC_PLAYER_HP_PACKET
{
	unsigned char size;
	char type;
	char player_id;
	short hp;
};

struct SC_ADD_ARROW_PACKET
{
	unsigned char size;
	char type;
	unsigned char arrow_id;
	float pos_x;
	float pos_z;
	float dir_x;
	float dir_z;
};

struct SC_MOVE_ARROW_PACKET
{
	unsigned char size;
	char type;
	unsigned char arrow_id;
	float pos_x;
	float pos_z;
};

struct SC_REMOVE_ARROW_PACKET
{
	unsigned char size;
	char type;
	unsigned char arrow_id;
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
	MONSTER_TYPE monster_type;
	short hp;
	float pos_x;		// 몬스터 위치
	float pos_z;
	float dir_x;		// 현재 보고있는 방향
	float dir_z;
};

struct SC_MOVE_MONSTER_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	float pos_x;		// 몬스터 위치
	float pos_z;
	float dir_x;		// 현재 보고있는 방향
	float dir_z;
};

struct SC_MONSTER_LOOK_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	float dir_x;		// 현재 보고있는 방향
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

struct SC_MONSTER_HP_PACKET
{
	unsigned char size;
	char type;
	char monster_id;
	short hp;
};

struct SC_MOVE_BOSS_PACKET
{
	unsigned char size;
	char type;
	float pos_x;
	float pos_z;
	float dir_x;
	float dir_z;
};

struct SC_BOSS_STATE_PACKET
{
	unsigned char size;
	char type;
	BOSS_STATE state;
};

struct SC_BOSS_HP_PACKET
{
	unsigned char size;
	char type;
	short hp;
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

