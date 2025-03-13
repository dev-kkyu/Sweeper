#pragma once

#include "VulkanFramework.h"

class SceneBase
{
public:
	enum class SCENE_TYPE : char {
		ERROR = 0, START = 1, LOBBY = 2, INGAME = 3
	};

protected:
	SCENE_TYPE sceneType;
	bool isEnd;

public:
	SceneBase();
	virtual ~SceneBase();

	virtual void enter() = 0;
	virtual void exit() = 0;

	virtual void onFramebufferResize(VkExtent2D framebufferExtent) = 0;

	virtual void update(float elapsedTime, uint32_t currentFrame) = 0;

	virtual void drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) = 0;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) = 0;

	virtual void processKeyboard(int key, int action, int mods) = 0;
	virtual void processMouseButton(int button, int action, int mods, float xpos, float ypos) = 0;
	virtual void processMouseScroll(double xoffset, double yoffset) = 0;
	virtual void processMouseCursor(float xpos, float ypos) = 0;

	// 네트워크 패킷 처리
	virtual void processPacket(unsigned char* packet) = 0;

	// 씬 종료 조건
	virtual bool getIsEnd() const = 0;
	// 씬 타입
	virtual SCENE_TYPE getSceneType() const final;

};

