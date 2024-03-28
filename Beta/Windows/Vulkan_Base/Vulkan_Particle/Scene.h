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
		VkPipeline particle = VK_NULL_HANDLE;
	} pipeline;

	vkf::BufferObject uniformBufferObject;

	VkDescriptorPool samplerDescriptorPool;

	vkf::MeshBuffer plainBuffer;
	vkf::Texture plainTexture;
	OBJModelObject* plainObject;

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

	// particleData
	VkBuffer particleVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory particleVertexBufferMemory = VK_NULL_HANDLE;
	uint32_t particleVertexCount;
	vkf::Texture particleTexture;
	float e_time = 0.f;

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	// obj 모델은 직접 buffer와 texture를 넣어주도록 설계. 따라서 texture를 위한 descriptor pool을 만들어준다.
	void createSamplerDescriptorPool(uint32_t setCount);

	void createParticle(int particleCount);

};

struct ParticleData {
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec2 emitDir;
	float emitTime;

	ParticleData(glm::vec3 pos, glm::vec2 texCoord, glm::vec2 emitDir, float emitTime)
		: pos{ pos }, texCoord{ texCoord }, emitDir{ emitDir }, emitTime{ emitTime } {}

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};