#pragma once

#include "RotateObject.h"

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

	vkf::Buffer buffer;
	vkf::Texture texture;

	GameObject* object;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouse(int button, int action, int mods);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createUniformBuffers();
	void createUboDescriptorPool();
	void createUboDescriptorSets();

};

struct UniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
