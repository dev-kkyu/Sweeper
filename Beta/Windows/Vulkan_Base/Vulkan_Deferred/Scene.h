#pragma once

#include "OBJModelObject.h"
#include "PlayerObject.h"
//#include "GLTFModelObject.h"
//#include "GLTFSkinModelObject.h"	// included PlayerObject
#include "Camera.h"

class Scene
{
private:
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	VkRenderPass& renderPass;

	struct {
		VkDescriptorSetLayout ubo = VK_NULL_HANDLE;
		VkDescriptorSetLayout sampler = VK_NULL_HANDLE;
		VkDescriptorSetLayout shadowUbo = VK_NULL_HANDLE;
		VkDescriptorSetLayout shadowMap = VK_NULL_HANDLE;
		VkDescriptorSetLayout ssbo = VK_NULL_HANDLE;
	} descriptorSetLayout;

	struct {
		VkPipelineLayout offscreen = VK_NULL_HANDLE;
		VkPipelineLayout model = VK_NULL_HANDLE;
		VkPipelineLayout skinModel = VK_NULL_HANDLE;
	} pipelineLayout;

	struct {
		VkPipeline offscreen = VK_NULL_HANDLE;
		VkPipeline model = VK_NULL_HANDLE;
		VkPipeline skinModel = VK_NULL_HANDLE;
	} pipeline;

	vkf::BufferObject uniformBufferObject;
	vkf::BufferObject shadowUniformBufferObject;

	vkf::OffscreenPass &offscreenPass;

	VkDescriptorPool samplerDescriptorPool;

	// obj 맵
	vkf::MeshBuffer mapBuffer;
	vkf::Texture mapTexture;
	OBJModelObject* mapObject;

	// obj 전사
	vkf::MeshBuffer warriorBuffer;
	vkf::Texture warriorTexture;
	std::array<OBJModelObject*, 10> warriorObject;

	// gltf skin 버섯
	VulkanGLTFSkinModel mushroomModel;
	std::array<GLTFSkinModelObject*, 100> mushroomObject;

	// gltf skin 임시 샘플
	VulkanGLTFSkinModel playerModel;
	PlayerObject* pPlayer;

	Camera camera;

	glm::vec3 lightPos = glm::vec3(30.f, 30.f, 30.f);

	unsigned int keyState;
	bool leftButtonPressed = false;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass, vkf::OffscreenPass& offscreen);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void setDescriptors();

	// obj 모델은 직접 buffer와 texture를 넣어주도록 설계. 따라서 texture를 위한 descriptor pool을 만들어준다.
	void createSamplerDescriptorPool(uint32_t setCount);

};