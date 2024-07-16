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

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;		// model 파이프라인은 0, 1, 2번 셋만 사용한다

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
		// 바운딩 박스 파이프라인
		VkPipeline boundingBoxPipeline;
		// 쿼드 백그라운드 파이프라인
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
	float sceneElapsedTime = 0.f;	// 현재는 구름 애니메이션에 사용

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

	// gltf 일반 맵
	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	// gltf 일반 화살들
	VulkanGLTFModel arrowModel;
	std::unordered_map<int, std::shared_ptr<ArrowObject>> pArrowObjects;

	// gltf skin 몬스터 모델들
	std::array<VulkanGLTFSkinModel, 4> monsterModel;	// 몬스터 종류 4가지
	std::unordered_map<int, std::shared_ptr<MonsterObject>> pMonsterObjects;	// 다형성을 위한 포인터 사용

	VulkanGLTFSkinModel bossModel;
	GLTFSkinModelObject bossObject;

	// gltf skin 캐릭터 에셋
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// 캐릭터 종류는 총 4개이다.
	PLAYER_TYPE player_type;							// 플레이어 타입 (4가지 종류, 서버의 protocol에 정의)
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] 와 같은 객체를 가리키도록 한다.
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

	// 네트워크 패킷 처리
	void processPacket(unsigned char* packet);

	PLAYER_TYPE getPlayerType() const;

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	void createSamplerDescriptorPool(uint32_t setCount);

};
