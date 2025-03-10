#include "StartScene.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "ResourceManager.h"

StartScene::StartScene(vkf::Device& fDevice)
	: fDevice{ fDevice }
{
	createSamplerDescriptorPool(2);		// 텍스처 두개

	texture[0].loadFromFile(fDevice, "models/Textures/startscene1.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);
	texture[1].loadFromFile(fDevice, "models/Textures/startscene2.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);

	isEnd = false;
}

StartScene::~StartScene()
{
	texture[0].destroy();
	texture[1].destroy();
	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);
}

void StartScene::enter()
{
	isEnd = false;
}

void StartScene::exit()
{
}

void StartScene::update(float elapsedTime, uint32_t currentFrame)
{
	sceneElapsedTime += elapsedTime;
}

void StartScene::drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void StartScene::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().quad);

	int texIndex = static_cast<int>(glm::fract(sceneElapsedTime) * 2.f);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &texture[texIndex].samplerDescriptorSet, 0, nullptr);

	glm::mat4 matrix{ 1.f };
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void StartScene::processKeyboard(int key, int action, int mods)
{
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_SPACE:
			isEnd = true;
			break;
		}
		break;
	}
}

void StartScene::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
}

void StartScene::processMouseScroll(double xoffset, double yoffset)
{
}

void StartScene::processMouseCursor(float xpos, float ypos)
{
}

void StartScene::processPacket(unsigned char* packet)
{
}

bool StartScene::getIsEnd() const
{
	return isEnd;
}

void StartScene::createSamplerDescriptorPool(uint32_t setCount)
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = setCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = setCount;

	if (vkCreateDescriptorPool(fDevice.logicalDevice, &poolInfo, nullptr, &samplerDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
