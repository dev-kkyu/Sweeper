#pragma once

#include "GLTFSkinModelObject.h"
#include "NetworkManager.h"

class LobbyScene
{
private:
	bool isEnd;
	PLAYER_TYPE selPlayerType;

	vkf::BufferObject uniformBufferObject;
	VkDescriptorSet shadowSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline skinModelPipeline;

	std::array<GLTFSkinModelObject, 4> playerObjects;

public:
	LobbyScene(vkf::Device& fDevice, std::array<VulkanGLTFSkinModel, 4>& playerModel, VkDescriptorSetLayout uboDescriptorSetLayout,
		VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline skinModelPipeline);
	~LobbyScene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);

	PLAYER_TYPE getPlayerType() const;

	bool getIsEnd() const;

};


