#pragma once

#include <unordered_map>

//#include "OBJModelObject.h"
#include "MonsterObject.h"
#include "ArrowObject.h"
//#include "GLTFModelObject.h"		// included ArrowObject;
//#include "GLTFSkinModelObject.h"	// included MonsterObject
#include "Camera.h"

#include "NetworkManager.h"

class PlayerObject;
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
		// ���� ��׶��� ����������
		VkPipeline cloudPipeline;
	} pipeline;

	VkDescriptorPool sceneSamplerDescriptorPool;
	struct {
		vkf::Effect warrior;
		vkf::Effect archor;
		struct {
			vkf::Effect attack;
			vkf::Effect skill;
		} mage;
		vkf::Effect arrow;
	} effect;
	vkf::Texture cloudTexture;
	float sceneElapsedTime = 0.f;	// ����� ���� �ִϸ��̼ǿ� ���

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

	// gltf �Ϲ� ��
	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	// gltf �Ϲ� ȭ���
	VulkanGLTFModel arrowModel;
	std::unordered_map<int, std::shared_ptr<ArrowObject>> pArrowObjects;

	// gltf skin ���� �𵨵�
	std::array<VulkanGLTFSkinModel, 4> monsterModel;	// ���� ���� 4����
	std::unordered_map<int, std::shared_ptr<MonsterObject>> pMonsterObjects;	// �������� ���� ������ ���

	VulkanGLTFSkinModel bossModel;
	GLTFSkinModelObject bossObject;

	// gltf skin ĳ���� ����
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// ĳ���� ������ �� 4���̴�.
	PLAYER_TYPE player_type;							// �÷��̾� Ÿ�� (4���� ����, ������ protocol�� ����)
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] �� ���� ��ü�� ����Ű���� �Ѵ�.
	std::array<std::shared_ptr<PlayerObject>, 4> pPlayers;
	int my_id = -1;

	Camera camera;

	unsigned int keyState;
	bool middleButtonPressed = false;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet, int& width, int& height);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen);
	void drawUI(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void drawEffect(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void drawBoundingBox(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseScroll(double xoffset, double yoffset);
	void processMouseCursor(float xpos, float ypos);

	// ��Ʈ��ũ ��Ŷ ó��
	void processPacket(unsigned char* packet);

	PLAYER_TYPE getPlayerType() const;

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	void createSamplerDescriptorPool(uint32_t setCount);

};
