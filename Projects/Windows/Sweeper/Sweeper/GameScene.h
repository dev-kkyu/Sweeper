#pragma once

#include <unordered_map>

//#include "OBJModelObject.h"
#include "BossObject.h"
#include "GLTFModelObject.h"		// for mapObject
//#include "GLTFSkinModelObject.h"	// included BossObject
#include "Camera.h"

#include "NetworkManager.h"

class PlayerObject;
class MonsterObject;
class ArrowObject;
class GameScene
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
			ScenePipeline sceneOnOff[2];
			struct {
				ScenePipeline scene;
				ScenePipeline offscreen;
			};
		};
		// �ٿ�� �ڽ� ����������
		VkPipeline boundingBoxPipeline;
		// ü�� �� UI ����������
		VkPipeline hpBarPipeline;
		VkPipeline bossHpBarPipeline;
		// ���� ��׶��� ����������
		VkPipeline cloudPipeline;
	} pipeline;

	VkDescriptorPool sceneSamplerDescriptorPool;
	struct {
		vkf::Effect warrior;
		vkf::Effect archer;
		vkf::Effect healer;
		struct {
			vkf::Effect attack;
			vkf::Effect skill;
		} mage;
		vkf::Effect arrow;
		vkf::Effect boss;
	} effect;
	vkf::Texture cloudTexture;
	float sceneElapsedTime = 0.f;	// ����� ���� �ִϸ��̼ǿ� ���

	struct {
		union {
			vkf::BufferObject uboOnOff[2];
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
	std::unordered_map<int, std::shared_ptr<ArrowObject>> pArrowObjects;		// �⺻ �����ڸ� �Ⱦ����� ptr�� �ϴ� ���� ���ϴ�

	// gltf skin ���� �𵨵�
	std::array<VulkanGLTFSkinModel, 4> monsterModel;	// ���� ���� 4����
	std::unordered_map<int, std::shared_ptr<MonsterObject>> pMonsterObjects;	// �������� ���� ������ ���

	VulkanGLTFSkinModel bossModel;
	std::unique_ptr<BossObject> pBossObject;

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
	GameScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet, int& width, int& height);
	~GameScene();

	void start(PLAYER_TYPE player_type);

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

	// Lobby Scene���� ������ �� �����
	std::array<VulkanGLTFSkinModel, 4>& getPlayerModel();
	VkDescriptorSetLayout getUBODescriptorSetLayout() const;
	VkDescriptorSetLayout getSamplerDescriptorSetLayout() const;
	VkDescriptorSetLayout getSSBODescriptorSetLayout() const;
	VkPipelineLayout getPipelineLayout() const;
	VkPipeline getModelPipeline() const;
	VkPipeline getSkinModelPipeline() const;
	VkPipeline getOffscreenModelPipeline() const;
	VkPipeline getOffscreenSkinModelPipeline() const;

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	void createSamplerDescriptorPool(uint32_t setCount);

};
