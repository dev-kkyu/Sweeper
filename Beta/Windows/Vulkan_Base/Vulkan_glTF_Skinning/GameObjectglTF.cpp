#include "GameObjectglTF.h"

GameObjectglTF::GameObjectglTF()
{
}

GameObjectglTF::~GameObjectglTF()
{
}

void GameObjectglTF::initialize()
{
}

void GameObjectglTF::update(float elapsedTime, uint32_t currentFrame)
{
}

void GameObjectglTF::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (model)
		model->draw(commandBuffer, pipelineLayout, modelTransform);
}

void GameObjectglTF::release()
{
}

void GameObjectglTF::setModel(VulkanglTFModel& model)
{
	this->model = &model;
}
