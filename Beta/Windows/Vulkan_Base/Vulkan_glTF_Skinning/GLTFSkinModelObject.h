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

public:
	GLTFSkinModelObject();
	virtual ~GLTFSkinModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setModel(VulkanGLTFSkinModel& model);

private:
	std::shared_ptr<Node>	findNode(std::shared_ptr<Node> parent, uint32_t index);
	std::shared_ptr<Node>	nodeFromIndex(uint32_t index);
	void					loadSkins();
	void					loadAnimations();
	void					loadNode(const tinygltf::Node& inputNode, std::shared_ptr<Node> parent, uint32_t nodeIndex, uint32_t& indexBufferCount);
	glm::mat4				getNodeMatrix(std::shared_ptr<Node> node);
	void					updateJoints(std::shared_ptr<Node> node);
	void					updateAnimation(float elapsedTime);

};

