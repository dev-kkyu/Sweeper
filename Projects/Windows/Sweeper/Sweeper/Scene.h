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

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;		// model ������������ 0, 1, 2�� �¸� ����Ѵ�

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

	// gltf �Ϲ� ��
	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	VulkanGLTFSkinModel mushroomModel;
	std::unordered_map<int, std::shared_ptr<GLTFSkinModelObject>> pMonsterObjects;	// �������� ���� ������ ���

	// gltf skin ĳ���� ����
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// ĳ���� ������ �� 4���̴�.
	PLAYER_TYPE player_type;							// �÷��̾� Ÿ�� (4���� ����, ������ protocol�� ����)
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] �� ���� ��ü�� ����Ű���� �Ѵ�.
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

	// ��Ʈ��ũ ��Ŷ ó��
	void processPacket(unsigned char* packet);

	PLAYER_TYPE getPlayerType() const;

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

};
