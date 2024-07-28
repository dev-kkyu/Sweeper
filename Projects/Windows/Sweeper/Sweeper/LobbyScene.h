#pragma once

#include "GLTFModelObject.h"
#include "GLTFSkinModelObject.h"
#include "NetworkManager.h"

class LobbyScene
{
private:
	bool isEnd;
	PLAYER_TYPE selPlayerType;

	glm::vec3 lightPos;		// 모델 그려줄 때 사용할 조명

	// 받아오는 것
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;

	VkExtent2D& framebufferExtent;

	VkDescriptorSet shadowSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline modelPipeline;
	VkPipeline skinModelPipeline;

	// 만드는 것
	vkf::BufferObject uniformBufferObject;
	vkf::BufferObject offscreenUniformBufferObject;

	VkPipeline buttonPipeline;

	VkDescriptorPool samplerDescriptorPool;

	vkf::Texture button[4];		// 전사, 궁수, 법사, 사제
	vkf::Texture startButton;	// 게임 시작

	VulkanGLTFModel podiumModel;
	GLTFModelObject podiumObject;

	std::array<GLTFSkinModelObject, 4> playerObjects;

public:
	LobbyScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkExtent2D& framebufferExtent,
		std::array<VulkanGLTFSkinModel, 4>& playerModel,
		VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout,
		VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline modelPipeline, VkPipeline skinModelPipeline);
	~LobbyScene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame, VkPipeline offscreenModelPipeline, VkPipeline offscreenSkinModelPipeline);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);

	PLAYER_TYPE getPlayerType() const;

	bool getIsEnd() const;

private:
	void createGraphicsPipeline();
	void createSamplerDescriptorPool(uint32_t setCount);

};


