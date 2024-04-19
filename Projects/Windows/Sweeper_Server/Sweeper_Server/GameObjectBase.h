#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "includes/glm/glm.hpp"
#include "includes/glm/gtc/matrix_transform.hpp"

class GameObjectBase
{
protected:
	// up은 항상 (0.f, 1.f, 0.f)이라고 가정한다.
	glm::mat4 modelTransform{ 1.f };
	glm::vec3 scaleValue{ 1.f };

	// 충돌 반지름 (기본 0.5f)
	float collisionRadius = 0.5f;

public:
	GameObjectBase();
	virtual ~GameObjectBase();

	virtual void initialize() = 0;
	virtual bool update(float elapsedTime) = 0;		// 업데이트 된 것이 없으면 false
	virtual void release() = 0;

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;					// y는 항상 0으로 가정
	virtual void setScale(glm::vec3 scale) final;
	virtual glm::vec3 getPosition() const final;
	virtual glm::vec3 getLook() const final;
	virtual glm::vec3 getRight() const final;

	virtual void moveForward(float value) final;
	virtual void move(glm::vec3 direction, float value) final;

	virtual void rotate(float degree) final;

	virtual bool isCollide(const GameObjectBase& other) final;

};
