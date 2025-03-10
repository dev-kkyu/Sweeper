#include "SceneManager.h"

#include "SoundManager.h"

#include "StartScene.h"
#include "LobbyScene.h"
#include "GameScene.h"

SceneManager::SceneManager(vkf::Device& fDevice, const VkExtent2D& framebufferExtent)
	: fDevice{ fDevice }, framebufferExtent{ framebufferExtent }
{
	nowScene = SCENE_TYPE::START;

	initScene();

}

SceneManager::~SceneManager()
{
}

void SceneManager::update(float elapsedTime, uint32_t currentFrame)
{
	if (pScene->getIsEnd()) {
		pScene->exit();

		switch (nowScene) {
		case SceneManager::SCENE_TYPE::START:
			nowScene = SCENE_TYPE::LOBBY;
			pScene = pLobbyScene;
			break;
		case SceneManager::SCENE_TYPE::LOBBY:
			NetworkManager::getInstance().connectServer();							// 서버 연결
			NetworkManager::getInstance().start(pLobbyScene->getPlayerType());		// 로그인 및 Recv 시작
			SoundManager::getInstance().playBGM();									// BGM 시작

			pGameScene->setPlayerType(pLobbyScene->getPlayerType());				// 로비에서 선택한 플레이어로 시작
			nowScene = SCENE_TYPE::INGAME;
			pScene = pGameScene;
			break;
		case SceneManager::SCENE_TYPE::INGAME:
			SoundManager::getInstance().stopBGM();									// BGM 종료
			NetworkManager::getInstance().stop();									// 서버 연결 종료

			nowScene = SCENE_TYPE::LOBBY;
			pScene = pLobbyScene;
			break;
		}

		pScene->enter();
	}

	pScene->update(elapsedTime, currentFrame);
}

void SceneManager::drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	pScene->drawOffscreen(commandBuffer, pipelineLayout, currentFrame);
}

void SceneManager::drawScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	pScene->draw(commandBuffer, pipelineLayout, currentFrame);
}

void SceneManager::processKeyboard(int key, int action, int mods)
{
	pScene->processKeyboard(key, action, mods);
}

void SceneManager::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	pScene->processMouseButton(button, action, mods, xpos, ypos);
}

void SceneManager::processMouseScroll(double xoffset, double yoffset)
{
	pScene->processMouseScroll(xoffset, yoffset);
}

void SceneManager::processMouseCursor(float xpos, float ypos)
{
	pScene->processMouseCursor(xpos, ypos);
}

void SceneManager::processPacket(unsigned char* packet)
{
	pScene->processPacket(packet);
}

void SceneManager::initScene()
{
	pGameScene = std::make_shared<GameScene>(fDevice, framebufferExtent);

	// GameScene 생성 후 생성 가능
	pStartScene = std::make_shared<StartScene>(fDevice);
	// GameScene 생성 후 생성 가능
	pLobbyScene = std::make_shared<LobbyScene>(fDevice, framebufferExtent, pGameScene->getPlayerModel());

	pScene = pStartScene;
}
