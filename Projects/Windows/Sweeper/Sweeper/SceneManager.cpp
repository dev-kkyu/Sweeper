#include "SceneManager.h"

#include "SoundManager.h"

#include "StartScene.h"
#include "LobbyScene.h"
#include "GameScene.h"

#include <stdexcept>

SceneManager::SceneManager(vkf::Device& fDevice, const VkExtent2D& framebufferExtent)
{
	pStartScene = std::make_shared<StartScene>(fDevice);
	pLobbyScene = std::make_shared<LobbyScene>(fDevice, framebufferExtent);
	pGameScene = std::make_shared<GameScene>(fDevice, framebufferExtent);

	pNowScene = pStartScene;
}

SceneManager::~SceneManager()
{
}

void SceneManager::onFramebufferResize(VkExtent2D framebufferExtent)
{
	pStartScene->onFramebufferResize(framebufferExtent);
	pLobbyScene->onFramebufferResize(framebufferExtent);
	pGameScene->onFramebufferResize(framebufferExtent);
}

void SceneManager::update(float elapsedTime, uint32_t currentFrame)
{
	if (pNowScene->getIsEnd()) {
		pNowScene->exit();

		switch (pNowScene->getSceneType()) {
		case SceneBase::SCENE_TYPE::START:
			pNowScene = pLobbyScene;
			break;
		case SceneBase::SCENE_TYPE::LOBBY:
			NetworkManager::getInstance().connectServer();							// 서버 연결
			NetworkManager::getInstance().start(pLobbyScene->getPlayerType());		// 로그인 및 Recv 시작
			SoundManager::getInstance().playBGM();									// BGM 시작

			pGameScene->setPlayerType(pLobbyScene->getPlayerType());				// 로비에서 선택한 플레이어로 시작
			pNowScene = pGameScene;
			break;
		case SceneBase::SCENE_TYPE::INGAME:
			SoundManager::getInstance().stopBGM();									// BGM 종료
			NetworkManager::getInstance().stop();									// 서버 연결 종료

			pNowScene = pLobbyScene;
			break;
		default:
			throw std::runtime_error("SCENE ERROR : INVALID TYPE!\n");
			break;
		}

		pNowScene->enter();
	}

	pNowScene->update(elapsedTime, currentFrame);
}

void SceneManager::drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	pNowScene->drawOffscreen(commandBuffer, pipelineLayout, currentFrame);
}

void SceneManager::drawScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	pNowScene->draw(commandBuffer, pipelineLayout, currentFrame);
}

void SceneManager::processKeyboard(int key, int action, int mods)
{
	pNowScene->processKeyboard(key, action, mods);
}

void SceneManager::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	pNowScene->processMouseButton(button, action, mods, xpos, ypos);
}

void SceneManager::processMouseScroll(double xoffset, double yoffset)
{
	pNowScene->processMouseScroll(xoffset, yoffset);
}

void SceneManager::processMouseCursor(float xpos, float ypos)
{
	pNowScene->processMouseCursor(xpos, ypos);
}

void SceneManager::processPacket(unsigned char* packet)
{
	pNowScene->processPacket(packet);
}
