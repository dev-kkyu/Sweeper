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

void GameObjectglTF::update(float elapsedTime)
{
}

void GameObjectglTF::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	if (model)
		model->draw(commandBuffer, pipelineLayout, modelTransform);
}

void GameObjectglTF::release()
{
}

void GameObjectglTF::setModel(VulkanGLTFModel& model)
{
	this->model = &model;
}
