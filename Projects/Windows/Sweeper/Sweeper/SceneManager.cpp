#include "SceneManager.h"

SceneManager::SceneManager(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet, int& winWidth, int& winHeight)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, shadowSetLayout{ shadowSetLayout }, shadowSet{ shadowSet }, winWidth{ winWidth }, winHeight{ winHeight }
{
	nowScene = SCENE_TYPE::START;

	isDrawBoundingBox = false;

	initScene();

}

SceneManager::~SceneManager()
{
}

void SceneManager::update(float elapsedTime, uint32_t currentFrame)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		pStartScene->update(elapsedTime, currentFrame);
		if (pStartScene->getIsEnd()) {
			nowScene = SCENE_TYPE::LOBBY;
		}
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		pLobbyScene->update(elapsedTime, currentFrame);
		if (pLobbyScene->getIsEnd()) {
			NetworkManager::getInstance().start(pLobbyScene->getPlayerType());		// 로그인 및 Recv 시작
			pGameScene->start(pLobbyScene->getPlayerType());
			nowScene = SCENE_TYPE::INGAME;
		}
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->update(elapsedTime, currentFrame);
		break;
	}
}

void SceneManager::drawOffscreen(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	if (SceneManager::SCENE_TYPE::INGAME == nowScene)			// mainScene에서만 offscreen 그린다
		pGameScene->draw(commandBuffer, currentFrame, true);
}

void SceneManager::drawScene(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		pStartScene->draw(commandBuffer, currentFrame);
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		pLobbyScene->draw(commandBuffer, currentFrame);
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->draw(commandBuffer, currentFrame, false);

		if (isDrawBoundingBox)
			pGameScene->drawBoundingBox(commandBuffer, currentFrame);

		pGameScene->drawEffect(commandBuffer, currentFrame);

		pGameScene->drawUI(commandBuffer, currentFrame);
		break;
	}
}

void SceneManager::processKeyboard(int key, int action, int mods)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		pStartScene->processKeyboard(key, action, mods);
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		pLobbyScene->processKeyboard(key, action, mods);
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->processKeyboard(key, action, mods);
		break;
	}
}

void SceneManager::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->processMouseButton(button, action, mods, xpos, ypos);
		break;
	}
}

void SceneManager::processMouseScroll(double xoffset, double yoffset)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->processMouseScroll(xoffset, yoffset);
		break;
	}
}

void SceneManager::processMouseCursor(float xpos, float ypos)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->processMouseCursor(xpos, ypos);
		break;
	}
}

void SceneManager::processPacket(unsigned char* packet)
{
	switch (nowScene)
	{
	case SceneManager::SCENE_TYPE::START:
		break;
	case SceneManager::SCENE_TYPE::LOBBY:
		break;
	case SceneManager::SCENE_TYPE::INGAME:
		pGameScene->processPacket(packet);
		break;
	}
}

void SceneManager::changeIsDrawBoundingBox()
{
	isDrawBoundingBox = not isDrawBoundingBox;
}

void SceneManager::initScene()
{
	pGameScene = std::make_unique<GameScene>(fDevice, msaaSamples, renderPass,
		shadowSetLayout, shadowSet, winWidth, winHeight);

	// GameScene 생성 후 생성 가능
	pStartScene = std::make_unique<StartScene>(fDevice, msaaSamples, renderPass,
		pGameScene->getSamplerDescriptorSetLayout(), pGameScene->getPipelineLayout());
	// GameScene 생성 후 생성 가능
	pLobbyScene = std::make_unique<LobbyScene>(fDevice, msaaSamples, renderPass, pGameScene->getPlayerModel(),
		pGameScene->getUBODescriptorSetLayout(), pGameScene->getSSBODescriptorSetLayout(), pGameScene->getSamplerDescriptorSetLayout(),
		shadowSet, pGameScene->getPipelineLayout(), pGameScene->getModelPipeline(), pGameScene->getSkinModelPipeline());

}
