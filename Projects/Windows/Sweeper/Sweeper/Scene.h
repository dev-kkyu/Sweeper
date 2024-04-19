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

	// obj 맵
	vkf::MeshBuffer mapBuffer;
	vkf::Texture mapTexture;
	OBJModelObject* mapObject;

	// obj 전사
	vkf::MeshBuffer warriorBuffer;
	vkf::Texture warriorTexture;
	std::array<OBJModelObject*, 10> warriorObject;

	// gltf 일반 도깨비불
	VulkanGLTFModel wispModel;
	std::array<GLTFModelObject*, 10> wispObject;

	VulkanGLTFSkinModel mushroomModel;
	std::unordered_map<int, std::shared_ptr<GLTFSkinModelObject>> pMonsterObjects;	// 다형성을 위한 포인터 사용

	// gltf skin 캐릭터 에셋
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// 캐릭터 종류는 총 4개이다.
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] 와 같은 객체를 가리키도록 한다.
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

	// 네트워크 패킷 처리
	void processPacket(unsigned char* packet);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	// obj 모델은 직접 buffer와 texture를 넣어주도록 설계. 따라서 texture를 위한 descriptor pool을 만들어준다.
	void createSamplerDescriptorPool(uint32_t setCount);

};
