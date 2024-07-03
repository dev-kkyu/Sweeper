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

	// �ٿ�� �ڽ��� ������ �´�
	for (const auto& node : this->model->nodes) {
		copyBoundingBoxByNode(node);
	}
}

void GLTFModelObject::updateBoundingBox()
{
	// �ٿ�� �ڽ� �ʱ�ȭ
	boundingBox.clear();
	// �ٿ�� �ڽ� ���� ����
	for (const auto& node : model->nodes) {
		copyBoundingBoxByNode(node);
	}
	// ���� ������Ʈ�� Ʈ�������� ����
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
		if (box.getTop() > 0.f)		// �Ʒ��� �ڽ��� ������� �ʴ´�.
			boundingBox.push_back(box);
	}

	for (const auto& child : node->children) {
		copyBoundingBoxByNode(child);
	}
}
