#pragma once

#include "GLTFModelObject.h"
#include "GLTFSkinModelObject.h"
#include "NetworkManager.h"

class LobbyScene
{
private:
	bool isEnd;
	PLAYER_TYPE selPlayerType;

	// �޾ƿ��� ��
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;

	VkDescriptorSet shadowSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline modelPipeline;
	VkPipeline skinModelPipeline;

	// ����� ��
	vkf::BufferObject uniformBufferObject;
	vkf::BufferObject offscreenUniformBufferObject;

	VkPipeline buttonPipeline;

	VkDescriptorPool samplerDescriptorPool;

	vkf::Texture button[4];		// ����, �ü�, ����, ����
	vkf::Texture startButton;	// ���� ����

	VulkanGLTFModel podiumModel;
	GLTFModelObject podiumObject;

	std::array<GLTFSkinModelObject, 4> playerObjects;

public:
	LobbyScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, std::array<VulkanGLTFSkinModel, 4>& playerModel,
		VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout,
		VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline modelPipeline, VkPipeline skinModelPipeline);
	~LobbyScene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame, VkPipeline offscreenModelPipeline, VkPipeline offscreenSkinModelPipeline);

	void processKeyboard(int key, int action, int mods);

	PLAYER_TYPE getPlayerType() const;

	bool getIsEnd() const;

private:
	void createGraphicsPipeline();
	void createSamplerDescriptorPool(uint32_t setCount);

};


