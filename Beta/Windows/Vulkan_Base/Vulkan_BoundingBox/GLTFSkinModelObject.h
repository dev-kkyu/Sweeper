#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFSkinModel.h"

class GLTFSkinModelObject : public GameObjectBase
{
	using Node = VulkanGLTFSkinModel::Node;
	using Skin = VulkanGLTFSkinModel::Skin;
	using Animation = VulkanGLTFSkinModel::Animation;

protected:
	VulkanGLTFSkinModel* model = nullptr;

	std::vector<std::shared_ptr<Node>>	nodes;
	std::vector<Skin>					skins;
	std::vector<Animation>				animations;

	uint32_t activeAnimation = 0;

	float animateSpeed = 1.f;

public:
	GLTFSkinModelObject();
	virtual ~GLTFSkinModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void initModel(VulkanGLTFSkinModel& model, VkDescriptorSetLayout ssboDescriptorSetLayout);

	void setAnimateSpeed(float speed);

	void changeAnimationClip();
	void setAnimationClip(uint32_t animationIndex);		// animation load 완료 후 호출해야 함.

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

