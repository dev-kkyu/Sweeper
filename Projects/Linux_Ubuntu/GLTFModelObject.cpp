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

	// 바운딩 박스를 복사해 온다
	for (const auto& node : this->model->nodes) {
		copyBoundingBoxByNode(node);
	}
}

void GLTFModelObject::updateBoundingBox()
{
	// 바운딩 박스 초기화
	boundingBox.clear();
	// 바운딩 박스 새로 복사
	for (const auto& node : model->nodes) {
		copyBoundingBoxByNode(node);
	}
	// 현재 오브젝트의 트랜스폼을 적용
	for (auto& box : boundingBox) {
		box.applyTransform(modelTransform);
	}
}

void GLTFModelObject::drawBoundingBox(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	for (const auto& box : boundingBox)
		box.draw(commandBuffer, pipelineLayout);
}

const std::vector<BoundingBox>& GLTFModelObject::getBoundingBox() const
{
	return boundingBox;
}

void GLTFModelObject::copyBoundingBoxByNode(const std::shared_ptr<VulkanGLTFModel::Node> node)
{
	for (const auto& box : node->mesh.boundingBox) {
		if (box.getTop() > 0.f)		// 아래쪽 박스는 사용하지 않는다.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
