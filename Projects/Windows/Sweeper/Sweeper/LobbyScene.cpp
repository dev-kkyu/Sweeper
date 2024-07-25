#include "LobbyScene.h"

#include <GLFW/glfw3.h>

LobbyScene::LobbyScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, std::array<VulkanGLTFSkinModel, 4>& playerModel,
	VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout,
	VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline modelPipeline, VkPipeline skinModelPipeline)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, shadowSet{ shadowSet },
	pipelineLayout{ pipelineLayout }, modelPipeline{ modelPipeline }, skinModelPipeline{ skinModelPipeline }
{
	uniformBufferObject.createUniformBufferObjects(fDevice, uboDescriptorSetLayout);
	offscreenUniformBufferObject.createUniformBufferObjects(fDevice, uboDescriptorSetLayout);

	podiumModel.loadModel(fDevice, samplerDescriptorSetLayout, "models/Character/Podium.glb");
	podiumObject.setModel(podiumModel);

	// 고정된 조명과 카메라
	vkf::UniformBufferObject ubo{};
	ubo.lightPos = glm::vec3(3.f, 5.f, 3.f);
	ubo.view = glm::lookAt(glm::vec3(0.f, 1.375f, 4.125f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.projection = glm::perspective(glm::radians(45.f), 16.f / 9.f, 1.f, 100.f);

	glm::mat4 lightView = glm::lookAt(ubo.lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 15.f);

	ubo.lightSpace = lightProj * lightView;
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {			// scene
		uniformBufferObject.updateUniformBuffer(ubo, i);
	}
	ubo.view = lightView;
	ubo.projection = lightProj;
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {			// offscreen
		offscreenUniformBufferObject.updateUniformBuffer(ubo, i);
	}

	float xPosition = 1.125f;
	podiumObject.setPosition(glm::vec3(xPosition, 0.f, 0.f));
	for (int i = 0; i < 4; ++i) {
		playerObjects[i].initModel(playerModel[i], ssboDescriptorSetLayout);
		playerObjects[i].setAnimationClip(23);
		playerObjects[i].setPosition(glm::vec3(xPosition, 0.f, 0.f));
	}

	isEnd = false;
	selPlayerType = PLAYER_TYPE::WARRIOR;
}

LobbyScene::~LobbyScene()
{
	podiumModel.destroy();

	uniformBufferObject.destroy();
}

void LobbyScene::update(float elapsedTime, uint32_t currentFrame)
{
	for (int i = 0; i < 4; ++i) {
		playerObjects[i].update(elapsedTime, currentFrame);
	}
}

void LobbyScene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	// shadow map bind
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &shadowSet, 0, nullptr);
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skinModelPipeline);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);
}

void LobbyScene::offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame, VkPipeline offscreenModelPipeline, VkPipeline offscreenSkinModelPipeline)
{
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &offscreenUniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenModelPipeline);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenSkinModelPipeline);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);
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

PLAYER_TYPE LobbyScene::getPlayerType() const
{
	return selPlayerType;
}

bool LobbyScene::getIsEnd() const
{
	return isEnd;
}
