#pragma once

#include "SceneBase.h"

class StartScene : public SceneBase
{
private:
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;

	VkPipeline pipeline;

	VkDescriptorPool samplerDescriptorPool;
	vkf::Texture texture[2];
	float sceneElapsedTime = 0.f;	// 텍스처 깜빡임을 위하여 사용

public:
	StartScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass);
	virtual ~StartScene();

	virtual void enter() override;
	virtual void exit() override;

	virtual void update(float elapsedTime, uint32_t currentFrame) override;

	virtual void drawOffscreen(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

	virtual void processKeyboard(int key, int action, int mods) override;
	virtual void processMouseButton(int button, int action, int mods, float xpos, float ypos) override;
	virtual void processMouseScroll(double xoffset, double yoffset) override;
	virtual void processMouseCursor(float xpos, float ypos) override;

	// 네트워크 패킷 처리
	virtual void processPacket(unsigned char* packet) override;

	// 씬 종료 조건
	virtual bool getIsEnd() const override;

private:
	void createGraphicsPipeline();
	void createSamplerDescriptorPool(uint32_t setCount);

};

