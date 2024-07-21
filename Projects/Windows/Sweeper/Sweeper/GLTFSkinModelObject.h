#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFSkinModel.h"

#include "BoundingBox.h"

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

	// 충돌 반지름 (기본 0.5f)
	float collisionRadius = 0.5f;

	short HP = 100;
	short maxHP = 100;

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
	uint32_t getAnimationClip() const;

	virtual void setCollisionRadius(float radius) final;
	virtual float getCollisionRadius() const final;
	BoundingBox getBoundingBox() const;
	void drawBoundingBox(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	void setHP(short HP);
	void setMaxHP(short maxHP);

private:
	std::shared_ptr<Node>	findNode(const std::shared_ptr<Node>& parent, uint32_t index) const;
	std::shared_ptr<Node>	nodeFromIndex(uint32_t index) const;
	void					loadSkins(VkDescriptorSetLayout ssboDescriptorSetLayout);
	void					loadAnimations();
	void					loadNode(const tinygltf::Node& inputNode, const std::shared_ptr<Node>& parent, uint32_t nodeIndex, uint32_t& indexBufferCount);
	glm::mat4				getNodeMatrix(const std::shared_ptr<Node>& node) const;
	void					updateJoints(const std::shared_ptr<Node>& node, uint32_t currentFrame);
	void					updateAnimation(float elapsedTime, uint32_t currentFrame);
	void					drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame, const std::shared_ptr<Node>& node, const glm::mat4& worldMatrix);

};
