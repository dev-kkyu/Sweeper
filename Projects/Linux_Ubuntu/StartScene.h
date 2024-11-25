#pragma once

#include "VulkanFramework.h"

class StartScene
{
private:
	bool isEnd;

	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	VkDescriptorPool samplerDescriptorPool;
	vkf::Texture texture[2];
	float sceneElapsedTime = 0.f;	// 텍스처 깜빡임을 위하여 사용

public:
	StartScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout samplerDescriptorSetLayout, VkPipelineLayout pipelineLayout);
	~StartScene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);

	bool getIsEnd() const;

private:
	void createGraphicsPipeline();
	void createSamplerDescriptorPool(uint32_t setCount);

};

