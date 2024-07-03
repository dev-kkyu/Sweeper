#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "includes/glm/glm.hpp"
#include "includes/glm/gtc/matrix_transform.hpp"

class BoundingBox
{
private:
	float top;
	float bottom;
	float front;
	float back;
	float left;
	float right;

public:
	BoundingBox();
	~BoundingBox();

	void setBound(float top, float bottom, float front, float back, float left, float right);
	void applyTransform(const glm::mat4& transform);	// 회전이 있는 matrix 사용시 주의할 것 (90도 단위는 괜찮다)

	bool isCollide(const BoundingBox& other) const;

	float getTop() const;
	float getBottom() const;
	float getFront() const;
	float getBack() const;
	float getLeft() const;
	float getRight() const;

};

