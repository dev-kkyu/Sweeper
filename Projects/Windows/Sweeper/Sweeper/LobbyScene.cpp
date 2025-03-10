#include "LobbyScene.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "ResourceManager.h"
#include "SoundManager.h"

LobbyScene::LobbyScene(vkf::Device& fDevice, const VkExtent2D& framebufferExtent)
	: fDevice{ fDevice }, framebufferExtent{ framebufferExtent }
{
	createSamplerDescriptorPool(5);		// 텍스처 5개

	uniformBufferObject.createUniformBufferObjects(fDevice, ResourceManager::getInstance().getDescriptorSetLayout().ubo);
	offscreenUniformBufferObject.createUniformBufferObjects(fDevice, ResourceManager::getInstance().getDescriptorSetLayout().ubo);

	button[static_cast<char>(PLAYER_TYPE::WARRIOR)].loadFromFile(fDevice, "models/Textures/Button/warrior.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);
	button[static_cast<char>(PLAYER_TYPE::ARCHER)].loadFromFile(fDevice, "models/Textures/Button/archer.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);
	button[static_cast<char>(PLAYER_TYPE::MAGE)].loadFromFile(fDevice, "models/Textures/Button/mage.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);
	button[static_cast<char>(PLAYER_TYPE::HEALER)].loadFromFile(fDevice, "models/Textures/Button/healer.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);
	startButton.loadFromFile(fDevice, "models/Textures/Button/gamestart.png", samplerDescriptorPool, ResourceManager::getInstance().getDescriptorSetLayout().sampler);

	podiumModel.loadModel(fDevice, ResourceManager::getInstance().getDescriptorSetLayout().sampler, "models/Character/Podium.glb");
	podiumObject.setModel(podiumModel);

	// 고정된 조명
	lightPos = glm::vec3(3.f, 5.f, 3.f);

	// offscreen ubo 값 고정
	vkf::UniformBufferObject ubo{};
	ubo.lightPos = lightPos;						// 안씀
	ubo.view = glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 15.f);
	ubo.lightSpace = ubo.projection * ubo.view;		// 안씀
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		offscreenUniformBufferObject.updateUniformBuffer(ubo, i);
	}

	float xPosition = 1.125f;
	podiumObject.setPosition(glm::vec3(xPosition, 0.f, 0.f));
	for (int i = 0; i < 4; ++i) {
		playerObjects[i].initModel(ResourceManager::getInstance().getPlayerModel()[i], ResourceManager::getInstance().getDescriptorSetLayout().ssbo);
		playerObjects[i].setAnimationClip(23);
		playerObjects[i].setPosition(glm::vec3(xPosition, 0.f, 0.f));
	}

	isEnd = false;
	selPlayerType = PLAYER_TYPE::WARRIOR;
}

LobbyScene::~LobbyScene()
{
	podiumModel.destroy();

	startButton.destroy();
	for (auto& texture : button) {
		texture.destroy();
	}

	offscreenUniformBufferObject.destroy();
	uniformBufferObject.destroy();

	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);
}

void LobbyScene::enter()
{
	isEnd = false;
	selPlayerType = PLAYER_TYPE::WARRIOR;
}

void LobbyScene::exit()
{
}

void LobbyScene::update(float elapsedTime, uint32_t currentFrame)
{
	// scene ubo 업데이트
	vkf::UniformBufferObject ubo{};
	ubo.lightPos = lightPos;
	ubo.view = glm::lookAt(glm::vec3(0.f, 1.375f, 4.125f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.projection = glm::perspective(glm::radians(45.f), float(framebufferExtent.width) / float(framebufferExtent.height), 1.f, 100.f);		// 종횡비에 알맞게
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 15.f);
	ubo.lightSpace = lightProj * lightView;
	uniformBufferObject.updateUniformBuffer(ubo, currentFrame);

	for (int i = 0; i < 4; ++i) {
		playerObjects[i].update(elapsedTime, currentFrame);
	}
}

void LobbyScene::drawOffscreen(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &offscreenUniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().offscreen.model);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().offscreen.skinModel);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);
}

void LobbyScene::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	// shadow map bind
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &ResourceManager::getInstance().getShadowDescriptorSet(), 0, nullptr);
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().scene.model);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().scene.skinModel);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);

	// UI 그려주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ResourceManager::getInstance().getPipeline().quad);
	// 플레이어 버튼
	for (int i = 0; i < 4; ++i) {
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &button[i].samplerDescriptorSet, 0, nullptr);
		glm::mat4 matrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.425f, 0.675f - i * 0.3f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height)), 0.08125f, 1.f));
		if (static_cast<int>(selPlayerType) != i)
			matrix[3][3] = 0.1f;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
	// 게임 시작 버튼
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &startButton.samplerDescriptorSet, 0, nullptr);
	glm::mat4 matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.625f, 0.f))
		* glm::scale(glm::mat4(1.f), glm::vec3(0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height)), 0.0875f, 1.f));
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void LobbyScene::processKeyboard(int key, int action, int mods)
{
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_0:
		case GLFW_KEY_KP_0:
			selPlayerType = PLAYER_TYPE::WARRIOR;
			break;
		case GLFW_KEY_1:
		case GLFW_KEY_KP_1:
			selPlayerType = PLAYER_TYPE::ARCHER;
			break;
		case GLFW_KEY_2:
		case GLFW_KEY_KP_2:
			selPlayerType = PLAYER_TYPE::MAGE;
			break;
		case GLFW_KEY_3:
		case GLFW_KEY_KP_3:
			selPlayerType = PLAYER_TYPE::HEALER;
			break;
		case GLFW_KEY_ENTER:
		case GLFW_KEY_KP_ENTER:
			isEnd = true;
			break;
		}
		break;
	}
}

void LobbyScene::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	switch (action) {
	case GLFW_RELEASE:
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			// 플레이어 선택
			for (int i = 0; i < 4; ++i) {
				// 그린 곳의 위치와 같게한다
				float bLeft = -1.f * (0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height))) - 0.425f;
				float bRight = 1.f * (0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height))) - 0.425f;
				float bTop = 1.f * 0.08125f + (0.675f - i * 0.3f);
				float bBottom = -1.f * 0.08125f + (0.675f - i * 0.3f);
				if (xpos > bLeft and xpos < bRight and ypos < bTop and ypos > bBottom) {	// 선택
					selPlayerType = static_cast<PLAYER_TYPE>(i);
					SoundManager::getInstance().playButtonSound();
					break;
				}
			}
			// 게임 시작 버튼 선택
			{
				float bLeft = -1.f * 0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height));
				float bRight = 1.f * 0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height));
				float bTop = 1.f * 0.0875f - 0.625f;
				float bBottom = -1.f * 0.0875f - 0.625f;
				if (xpos > bLeft and xpos < bRight and ypos < bTop and ypos > bBottom) {	// 선택
					SoundManager::getInstance().playButtonSound();
					isEnd = true;
					break;
				}
			}
			break;
		}
		break;
	}
}

void LobbyScene::processMouseScroll(double xoffset, double yoffset)
{
}

void LobbyScene::processMouseCursor(float xpos, float ypos)
{
}

void LobbyScene::processPacket(unsigned char* packet)
{
}

bool LobbyScene::getIsEnd() const
{
	return isEnd;
}

PLAYER_TYPE LobbyScene::getPlayerType() const
{
	return selPlayerType;
}

void LobbyScene::createSamplerDescriptorPool(uint32_t setCount)
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
