#pragma once

#include "VulkanFramework.h"

class GameObjectBase
{
protected:
	// up�� �׻� (0.f, 1.f, 0.f)�̶�� �����Ѵ�.
	glm::mat4 modelTransform{ 1.f };
	glm::vec3 scaleValue{ 1.f };

public:
	GameObjectBase();
	virtual ~GameObjectBase();

	virtual void initialize() = 0;
	virtual void update(float elapsedTime) = 0;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) = 0;
	virtual void release() = 0;

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;					// y�� �׻� 0���� ����
	virtual void setScale(glm::vec3 scale) final;
	virtual glm::vec3 getPosition() const final;
	virtual glm::vec3 getLook() const final;
	virtual glm::vec3 getRight() const final;

	virtual void moveForward(float value) final;
	virtual void move(glm::vec3 direction, float value) final;

	virtual void rotate(float degree) final;

};

struct PushConstantData {
	alignas(16) glm::mat4 model;
};
