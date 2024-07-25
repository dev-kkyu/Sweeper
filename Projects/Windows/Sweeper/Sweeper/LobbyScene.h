#pragma once

#include "GLTFSkinModelObject.h"
#include "NetworkManager.h"

class LobbyScene
{
private:
	bool isEnd;
	PLAYER_TYPE selPlayerType;

	// 받아오는 것
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;

	VkDescriptorSet shadowSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline modelPipeline;
	VkPipeline skinModelPipeline;

	// 만드는 것
	vkf::BufferObject uniformBufferObject;

	std::array<GLTFSkinModelObject, 4> playerObjects;

public:
	LobbyScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, std::array<VulkanGLTFSkinModel, 4>& playerModel,
		VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout,
		VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline modelPipeline, VkPipeline skinModelPipeline);
	~LobbyScene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);

	PLAYER_TYPE getPlayerType() const;

	bool getIsEnd() const;

};


