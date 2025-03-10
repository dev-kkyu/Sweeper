#pragma once

#include "SceneBase.h"

#include <unordered_map>

#include "BossObject.h"
#include "GLTFModelObject.h"		// for mapObject
//#include "GLTFSkinModelObject.h"	// included BossObject
#include "Camera.h"

#include "NetworkManager.h"

class PlayerObject;
class MonsterObject;
class ArrowObject;
class GameScene : public SceneBase
{
private:
	bool isEndPacketReceived;
	bool isWin;
	float gameEndAfterTime;

private:	// 사용자 입력에 따른 변수
	bool middleButtonPressed;
	bool isDrawingBoundingBox;

private:
	vkf::Device& fDevice;

	struct {
		// 바운딩 박스 파이프라인
		VkPipeline boundingBoxPipeline;
		// 체력 바 UI 파이프라인
		VkPipeline hpBarPipeline;
		VkPipeline bossHpBarPipeline;
		// 백그라운드 구름 파이프라인
		VkPipeline cloudPipeline;
		// 백그라운드 게임승리, 패배 파이프라인
		VkPipeline gameendPipeline;
		// 힐러 파티클 파이프라인
		VkPipeline particlePipeline;
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
	vkf::Texture gameendTexture[2];
	float sceneElapsedTime = 0.f;	// 현재는 구름 애니메이션에 사용

	struct {
		union {
			vkf::BufferObject uboOnOff[2];
			struct {
				vkf::BufferObject scene;
				vkf::BufferObject offscreen;
			};
		};
	} uniformBufferObject;

	// 힐러 스킬 발동시 대상 플레이어 주변에 띄울 파티클
	VkBuffer particleVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory particleVertexBufferMemory = VK_NULL_HANDLE;
	uint32_t particleVertexCount;
	vkf::Texture particleTexture;

	glm::vec3 lightPos = glm::vec3(10.f, 10.f, 10.f);

	// gltf 일반 맵
	VulkanGLTFModel mapModel;
	GLTFModelObject mapObject;

	// gltf 일반 화살들
	VulkanGLTFModel arrowModel;
	std::unordered_map<int, std::shared_ptr<ArrowObject>> pArrowObjects;		// 기본 생성자를 안쓰려면 ptr로 하는 것이 편하다

	// gltf skin 몬스터 모델들
	std::array<VulkanGLTFSkinModel, 4> monsterModel;	// 몬스터 종류 4가지
	std::unordered_map<int, std::shared_ptr<MonsterObject>> pMonsterObjects;	// 다형성을 위한 포인터 사용

	VulkanGLTFSkinModel bossModel;
	std::unique_ptr<BossObject> pBossObject;

	// gltf skin 캐릭터 에셋
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// 캐릭터 종류는 총 4개이다.
	PLAYER_TYPE playerType;								// 플레이어 타입 (4가지 종류, 서버의 protocol에 정의)
	std::shared_ptr<PlayerObject> pMyPlayer;			// pPlayers[my_id] 와 같은 객체를 가리키도록 한다.
	std::array<std::shared_ptr<PlayerObject>, 4> pPlayers;
	int my_id = -1;
	int observer_id = -1;

	Camera camera;

public:
	GameScene(vkf::Device& fDevice, const VkExtent2D& framebufferExtent);
	virtual ~GameScene();

	virtual void enter() override;
	virtual void exit() override;

	virtual void update(float elapsedTime, uint32_t currentFrame) override;

	virtual void drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

private:
	// draw에 offscreen을 분리하지 않기 위한 함수
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame, bool isOffscreen);
	void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void drawBoundingBox(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);

public:
	virtual void processKeyboard(int key, int action, int mods) override;
	virtual void processMouseButton(int button, int action, int mods, float xpos, float ypos) override;
	virtual void processMouseScroll(double xoffset, double yoffset) override;
	virtual void processMouseCursor(float xpos, float ypos) override;

	// 네트워크 패킷 처리
	virtual void processPacket(unsigned char* packet) override;

	// 씬 종료 조건
	virtual bool getIsEnd() const override;

	void setPlayerType(PLAYER_TYPE player_type);
	PLAYER_TYPE getPlayerType() const;

	// Lobby Scene에서 가져다 쓸 내용들
	std::array<VulkanGLTFSkinModel, 4>& getPlayerModel();

private:
	void createGraphicsPipeline();

	void createSamplerDescriptorPool(uint32_t setCount);

	// 힐러가 사용할 파티클. Scene에서 그려준다
	void createParticle(int particleCount);

};

struct HealerParticleData {
	glm::vec3 pos;
	float emitTime;
	float lifeTime;

	HealerParticleData(glm::vec3 pos, float emitTime, float lifeTime);

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
