#pragma once

#include "PlayerObject.h"
#include "GameObjectglTF.h"
#include "Camera.h"

class Scene
{
private:
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	VkRenderPass& renderPass;

	VkDescriptorSetLayout uboDescriptorSetLayout;
	VkDescriptorSetLayout samplerDescriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
	std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> uniformBuffersMemory;
	std::array<void*, MAX_FRAMES_IN_FLIGHT> uniformBuffersMapped;

	VkDescriptorPool uboDescriptorPool;
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> uboDescriptorSets;

	VkDescriptorPool samplerDescriptorPool;

	vkf::MeshBuffer plainBuffer;
	vkf::Texture plainTexture;
	GameObjectObj* plainObject;

	vkf::MeshBuffer boxBuffer;
	vkf::Texture boxTexture;
	PlayerObject* pPlayer;

	GameObjectglTF* sampleModelObject[2];
	VulkanGLTFModel sampleModel[2];

	Camera camera;

	unsigned int keyState;
	bool leftButtonPressed = false;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createUniformBuffers();
	void createUboDescriptorPool();
	void createUboDescriptorSets();

	void createSamplerDescriptorPool(uint32_t setCount);

};

struct UniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
