#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFSkinModel.h"

class ClipSkinModel;
class GLTFSkinModelObject : public GameObjectBase
{
protected:
	std::vector<ClipSkinModel> clipModels;

	uint32_t clipIndex = 0;

	float animateSpeed = 1.f;

public:
	GLTFSkinModelObject();
	virtual ~GLTFSkinModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void addModel(VulkanGLTFSkinModel& model, VkDescriptorSetLayout ssboDescriptorSetLayout);
	void selectClip(int index);
	void setAnimateSpeed(float speed);
};

class ClipSkinModel
{
private:
	using Node = VulkanGLTFSkinModel::Node;
	using Skin = VulkanGLTFSkinModel::Skin;
	using Animation = VulkanGLTFSkinModel::Animation;

	VulkanGLTFSkinModel& model;

	std::vector<std::shared_ptr<Node>>	nodes;
	std::vector<Skin>					skins;
	std::vector<Animation>				animations;

	uint32_t activeAnimation = 0;

public:
	ClipSkinModel(VulkanGLTFSkinModel& model, VkDescriptorSetLayout ssboDescriptorSetLayout);
	~ClipSkinModel();

	void update(float elapsedTime, uint32_t currentFrame, float animateSpeed);
	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame, const glm::mat4& worldMatrix);

private:
	std::shared_ptr<Node>	findNode(std::shared_ptr<Node> parent, uint32_t index);
	std::shared_ptr<Node>	nodeFromIndex(uint32_t index);
	void					loadSkins(VkDescriptorSetLayout ssboDescriptorSetLayout);
	void					loadAnimations();
	void					loadNode(const tinygltf::Node& inputNode, std::shared_ptr<Node> parent, uint32_t nodeIndex, uint32_t& indexBufferCount);
	glm::mat4				getNodeMatrix(std::shared_ptr<Node> node);
	void					updateJoints(std::shared_ptr<Node> node, uint32_t currentFrame);
	void					updateAnimation(float elapsedTime, uint32_t currentFrame);
	void					drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame, std::shared_ptr<Node> node, const glm::mat4& worldMatrix);

};
