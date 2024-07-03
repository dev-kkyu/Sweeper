#pragma once

#include "GameObjectBase.h"
#include "VulkanGLTFModel.h"

class GLTFModelObject : public GameObjectBase
{
protected:

	VulkanGLTFModel* model = nullptr;

	std::vector<BoundingBox> boundingBox;

public:
	GLTFModelObject();
	virtual ~GLTFModelObject();

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	void setModel(VulkanGLTFModel& model);

	void updateBoundingBox();
	void drawBoundingBox(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;
	const std::vector<BoundingBox>& getBoundingBox() const;

private:
	void copyBoundingBoxByNode(const std::shared_ptr<VulkanGLTFModel::Node> node);

};

