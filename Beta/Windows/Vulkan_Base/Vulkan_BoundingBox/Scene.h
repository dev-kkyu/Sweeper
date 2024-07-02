#pragma once

#include "PlayerObject.h"
#include "GLTFModelObject.h"
#include "GLTFSkinModelObject.h"
#include "Camera.h"

class Scene
{
private:
	vkf::Device& fDevice;
	VkSampleCountFlagBits& msaaSamples;
	vkf::RenderPass& renderPass;
	VkDescriptorSetLayout& shadowSetLayout;
	VkDescriptorSet& shadowSet;

	struct {
		VkDescriptorSetLayout ubo = VK_NULL_HANDLE;
		VkDescriptorSetLayout sampler = VK_NULL_HANDLE;
		VkDescriptorSetLayout ssbo = VK_NULL_HANDLE;
	} descriptorSetLayout;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;		// model ������������ 0, 1, 2�� �¸� ����Ѵ�

	struct ScenePipeline {
		VkPipeline model;
		VkPipeline skinModel;
	};
	struct {
		union {
			ScenePipeline sceneOnOff[2]{};
			struct {
				ScenePipeline scene;
				ScenePipeline offscreen;
			};
		};
		// �ٿ�� �ڽ� ����������
		VkPipeline boundingBoxPipeline;
	} pipeline;

	struct {
		union {
			vkf::BufferObject uboOnOff[2]{};
			struct {
				vkf::BufferObject scene;
				vkf::BufferObject offscreen;
			};
		};
	} uniformBufferObject;

	glm::vec3 lightPos = glm::vec3(10.f, 10.f, 10.f);

	VkDescriptorPool samplerDescriptorPool;

	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	vkf::MeshBuffer boxBuffer;
	vkf::Texture boxTexture;
	PlayerObject* pPlayer;

	GLTFModelObject* gltfModelObject;
	VulkanGLTFModel gltfModel;

	GLTFSkinModelObject* skinModelObject[2];
	VulkanGLTFSkinModel skinModel[2];

	Camera camera;

	unsigned int keyState;
	bool leftButtonPressed = false;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	// obj ���� ���� buffer�� texture�� �־��ֵ��� ����. ���� texture�� ���� descriptor pool�� ������ش�.
	void createSamplerDescriptorPool(uint32_t setCount);

};
