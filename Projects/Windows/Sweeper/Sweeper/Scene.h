#pragma once

#include <unordered_map>

#include "OBJModelObject.h"
#include "PlayerObject.h"
#include "GLTFModelObject.h"
//#include "GLTFSkinModelObject.h"	// included PlayerObject
#include "Camera.h"

#include "NetworkManager.h"

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

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;		// model 파이프라인은 0, 1, 2번 셋만 사용한다

	struct Pipeline {
		struct Type {
			VkPipeline model = VK_NULL_HANDLE;
			VkPipeline skinModel = VK_NULL_HANDLE;
		} scene, offscreen;
	} pipeline;

	struct {
		vkf::BufferObject scene;
		vkf::BufferObject offscreen;
	} uniformBufferObject;

	glm::vec3 lightPos = glm::vec3(10.f, 10.f, 10.f);

	// gltf 일반 맵
	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	VulkanGLTFSkinModel mushroomModel;
	std::unordered_map<int, std::shared_ptr<GLTFSkinModelObject>> pMonsterObjects;	// 다형성을 위한 포인터 사용

	// gltf skin 캐릭터 에셋
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// 캐릭터 종류는 총 4개이다.
	PLAYER_TYPE player_type;							// 플레이어 타입 (4가지 종류, 서버의 protocol에 정의)
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] 와 같은 객체를 가리키도록 한다.
	std::array<std::shared_ptr<PlayerObject>, 4> pPlayers;
	int my_id = -1;

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

	// 네트워크 패킷 처리
	void processPacket(unsigned char* packet);

	PLAYER_TYPE getPlayerType() const;

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

};
