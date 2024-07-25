#include "LobbyScene.h"

#include <GLFW/glfw3.h>

LobbyScene::LobbyScene()
{
	isEnd = false;
	selPlayerType = PLAYER_TYPE::WARRIOR;
}

LobbyScene::~LobbyScene()
{
}

void LobbyScene::processKeyboard(int key, int action, int mods)
{
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_0:
		case GLFW_KEY_KP_0:
			selPlayerType = PLAYER_TYPE::WARRIOR;
			break;
		case GLFW_KEY_1:
		case GLFW_KEY_KP_1:
			selPlayerType = PLAYER_TYPE::ARCHER;
			break;
		case GLFW_KEY_2:
		case GLFW_KEY_KP_2:
			selPlayerType = PLAYER_TYPE::MAGE;
			break;
		case GLFW_KEY_3:
		case GLFW_KEY_KP_3:
			selPlayerType = PLAYER_TYPE::HEALER;
			break;
		case GLFW_KEY_ENTER:
		case GLFW_KEY_KP_ENTER:
			isEnd = true;
			break;
		}
		break;
	}
}

PLAYER_TYPE LobbyScene::getPlayerType() const
{
	return selPlayerType;
}

bool LobbyScene::getIsEnd() const
{
	return isEnd;
}
