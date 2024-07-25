#pragma once

#include "LobbyScene.h"
#include "GameScene.h"

class SceneManager
{
private:
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;
	VkDescriptorSetLayout& shadowSetLayout;
	VkDescriptorSet& shadowSet;

	int& winWidth;
	int& winHeight;

private:
	enum class SCENE_TYPE {
		LOBBY = 0, ENTER = 1, INGAME = 2
	};
	SCENE_TYPE nowScene;

	bool isDrawBoundingBox;

	std::unique_ptr<LobbyScene> pLobbyScene;
	std::unique_ptr<GameScene> pGameScene;


public:
	SceneManager(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet, int& winWidth, int& winHeight);
	~SceneManager();

	void update(float elapsedTime, uint32_t currentFrame);

	void drawOffscreen(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void drawScene(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseScroll(double xoffset, double yoffset);
	void processMouseCursor(float xpos, float ypos);

	// 匙飘况农 菩哦 贸府
	void processPacket(unsigned char* packet);

	void changeIsDrawBoundingBox();

	//PLAYER_TYPE getPlayerType() const;

private:
	void initScene();


};

