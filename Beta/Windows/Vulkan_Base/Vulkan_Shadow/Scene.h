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
	vkf::RenderPass& renderPass;

	struct {
		VkDescriptorSetLayout ubo = VK_NULL_HANDLE;
		VkDescriptorSetLayout sampler = VK_NULL_HANDLE;
		VkDescriptorSetLayout ssbo = VK_NULL_HANDLE;
	} descriptorSetLayout;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;		// model ������������ 0, 1�� �¸� ����Ѵ�

	struct Pipeline {
		struct Type {
			VkPipeline model = VK_NULL_HANDLE;
			VkPipeline skinModel = VK_NULL_HANDLE;
		} scene, offscreen;
	} pipeline;

	struct {
		vkf::BufferObject scene;
		vkf::BufferObject offscreen;	// ���� ����ȭ �� �� �ִ� (shader �ٲٰ� �Ǹ� view, projection, lightPos �ʿ����)
	} uniformBufferObject;

	glm::vec3 lightPos = glm::vec3(5.f, 5.f, 5.f);

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

public:
	Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass);
	~Scene();

	void update(float elapsedTime, uint32_t currentFrame);
	void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen);

	void processKeyboard(int key, int action, int mods);
	void processMouseButton(int button, int action, int mods, float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

private:
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	// obj ���� ���� buffer�� texture�� �־��ֵ��� ����. ���� texture�� ���� descriptor pool�� ������ش�.
	void createSamplerDescriptorPool(uint32_t setCount);

};
