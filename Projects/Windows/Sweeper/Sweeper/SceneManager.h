#pragma once

#include <memory>

#include "VulkanFramework.h"

class SceneBase;
class StartScene;
class LobbyScene;
class GameScene;
class SceneManager
{
private:
	std::shared_ptr<SceneBase> pNowScene;

	std::shared_ptr<StartScene> pStartScene;
	std::shared_ptr<LobbyScene> pLobbyScene;
	std::shared_ptr<GameScene> pGameScene;

public:
	SceneManager(vkf::Device& fDevice, const VkExtent2D& framebufferExtent);
	~SceneManager();

	void update(float elapsedTime, uint32_t currentFrame);

	void drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void drawScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseScroll(double xoffset, double yoffset);
	void processMouseCursor(float xpos, float ypos);

	// 匙飘况农 菩哦 贸府
	void processPacket(unsigned char* packet);

};

