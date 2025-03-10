#pragma once

#include "SceneBase.h"
#include "GLTFModelObject.h"
#include "GLTFSkinModelObject.h"
#include "NetworkManager.h"

class LobbyScene : public SceneBase
{
private:
	PLAYER_TYPE selPlayerType;

	glm::vec3 lightPos;		// 모델 그려줄 때 사용할 조명

	// 받아오는 것
	vkf::Device& fDevice;

	const VkExtent2D& framebufferExtent;

	// 만드는 것
	vkf::BufferObject uniformBufferObject;
	vkf::BufferObject offscreenUniformBufferObject;

	VkDescriptorPool samplerDescriptorPool;

	vkf::Texture button[4];		// 전사, 궁수, 법사, 사제
	vkf::Texture startButton;	// 게임 시작

	VulkanGLTFModel podiumModel;
	GLTFModelObject podiumObject;

	std::array<GLTFSkinModelObject, 4> playerObjects;

public:
	LobbyScene(vkf::Device& fDevice, const VkExtent2D& framebufferExtent, std::array<VulkanGLTFSkinModel, 4>& playerModel);
	virtual ~LobbyScene();

	virtual void enter() override;
	virtual void exit() override;

	virtual void update(float elapsedTime, uint32_t currentFrame) override;

	virtual void drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;

	virtual void processKeyboard(int key, int action, int mods) override;
	virtual void processMouseButton(int button, int action, int mods, float xpos, float ypos) override;
	virtual void processMouseScroll(double xoffset, double yoffset) override;
	virtual void processMouseCursor(float xpos, float ypos) override;

	// 네트워크 패킷 처리
	virtual void processPacket(unsigned char* packet) override;

	// 씬 종료 조건
	virtual bool getIsEnd() const override;

	PLAYER_TYPE getPlayerType() const;

private:
	void createSamplerDescriptorPool(uint32_t setCount);

};


