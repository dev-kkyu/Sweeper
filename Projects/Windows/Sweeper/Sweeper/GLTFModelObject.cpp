#include "GLTFModelObject.h"

GLTFModelObject::GLTFModelObject()
{
}

GLTFModelObject::~GLTFModelObject()
{
}

void GLTFModelObject::initialize()
{
}

void GLTFModelObject::update(float elapsedTime, uint32_t currentFrame)
{
}

void GLTFModelObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (model)
		model->draw(commandBuffer, pipelineLayout, modelTransform);
}

void GLTFModelObject::release()
{
}

void GLTFModelObject::setModel(VulkanGLTFModel& model)
{
	this->model = &model;
}
