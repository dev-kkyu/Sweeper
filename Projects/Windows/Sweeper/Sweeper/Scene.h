#pragma once

#include <unordered_map>

#include "OBJModelObject.h"
#include "PlayerObject.h"
#include "GLTFModelObject.h"
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
		VkDescriptorSetLayout ssbo = VK_NULL_HANDLE;
	} descriptorSetLayout;

	struct {
		VkPipelineLayout model = VK_NULL_HANDLE;
		VkPipelineLayout skinModel = VK_NULL_HANDLE;
	} pipelineLayout;

	struct {
		VkPipeline model = VK_NULL_HANDLE;
		VkPipeline skinModel = VK_NULL_HANDLE;
	} pipeline;

	vkf::BufferObject uniformBufferObject;

	VkDescriptorPool samplerDescriptorPool;

	// obj ��
	vkf::MeshBuffer mapBuffer;
	vkf::Texture mapTexture;
	OBJModelObject* mapObject;

	// obj ����
	vkf::MeshBuffer warriorBuffer;
	vkf::Texture warriorTexture;
	std::array<OBJModelObject*, 10> warriorObject;

	// gltf �Ϲ� �������
	VulkanGLTFModel wispModel;
	std::array<GLTFModelObject*, 10> wispObject;

	VulkanGLTFSkinModel mushroomModel;
	std::unordered_map<int, std::shared_ptr<GLTFSkinModelObject>> pMonsterObjects;	// �������� ���� ������ ���

	// gltf skin ĳ���� ����
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// ĳ���� ������ �� 4���̴�.
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] �� ���� ��ü�� ����Ű���� �Ѵ�.
	std::array<std::shared_ptr<PlayerObject>, 4> pPlayers;
	int my_id = -1;

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

	// ��Ʈ��ũ ��Ŷ ó��
	void processPacket(unsigned char* packet);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	// obj ���� ���� buffer�� texture�� �־��ֵ��� ����. ���� texture�� ���� descriptor pool�� ������ش�.
	void createSamplerDescriptorPool(uint32_t setCount);

};
