#include "BoundingBox.h"

#include <tuple>

BoundingBox::BoundingBox()
{
	top = 1.f;
	bottom = 0.f;
	front = 0.5f;
	back = -0.5f;
	left = -0.5f;
	right = 0.5f;
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::setBound(float top, float bottom, float front, float back, float left, float right)
{
	this->top = top;
	this->bottom = bottom;
	this->front = front;
	this->back = back;
	this->left = left;
	this->right = right;
}

bool BoundingBox::isCollide(const BoundingBox& other) const
{
	if (left >= other.right) return false;
	if (right <= other.left) return false;
	if (top <= other.bottom) return false;
	if (bottom >= other.top) return false;
	if (front <= other.back) return false;
	if (back >= other.front) return false;
	return true;
}

float BoundingBox::getTop() const
{
	return top;
}

float BoundingBox::getBottom() const
{
	return bottom;
}

float BoundingBox::getFront() const
{
	return front;
}

float BoundingBox::getBack() const
{
	return back;
}

float BoundingBox::getLeft() const
{
	return left;
}

float BoundingBox::getRight() const
{
	return right;
}
