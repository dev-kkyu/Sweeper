#pragma once

#include "NetworkManager.h"

class LobbyScene
{
private:
	bool isEnd;
	PLAYER_TYPE selPlayerType;

public:
	LobbyScene();
	~LobbyScene();

	void processKeyboard(int key, int action, int mods);

	PLAYER_TYPE getPlayerType() const;

	bool getIsEnd() const;

};


