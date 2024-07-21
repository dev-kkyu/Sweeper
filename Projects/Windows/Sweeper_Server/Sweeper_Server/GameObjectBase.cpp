#include "GameObjectBase.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

GameObjectBase::GameObjectBase(Room* parentRoom, int o_id)
	: parentRoom{ parentRoom }, my_id{ o_id }
{
}

GameObjectBase::~GameObjectBase()
{
}

void GameObjectBase::setPosition(glm::vec3 position)
{
	modelTransform[3] = glm::vec4(position, 1.f);
}

void GameObjectBase::setLook(glm::vec3 look)
{
	glm::vec3 position = getPosition();

	glm::vec3 baseDir{ 0.f, 0.f, 1.f };
	glm::vec3 normalizedLook = glm::normalize(look);
	float radianAngle = glm::orientedAngle(baseDir, normalizedLook, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.f), radianAngle, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scaleValue);

	modelTransform = rotateMat * scaleMat;
	setPosition(position);
}

void GameObjectBase::setScale(glm::vec3 scale)
{
	scaleValue = scale;

	setLook(getLook());		// 동일한 작업을 이 함수에서 해준다.
}

glm::vec3 GameObjectBase::getPosition() const
{
	return glm::vec3(modelTransform[3]);
}

glm::vec3 GameObjectBase::getLook() const
{
	return glm::normalize(glm::vec3(modelTransform[2]));
}

glm::vec3 GameObjectBase::getRight() const
{
	return glm::normalize(glm::vec3(modelTransform[0]));
}

void GameObjectBase::moveForward(float value)
{
	auto position = getPosition();
	position += getLook() * value;
	setPosition(position);
}

void GameObjectBase::move(glm::vec3 direction, float value)
{
	if (glm::length(direction) > 0.f) {
		auto position = getPosition();
		position += glm::normalize(direction) * value;
		setPosition(position);
	}
}

void GameObjectBase::rotate(float degree)
{
	glm::vec3 originPos = getPosition();
	setPosition(glm::vec3(0.f));			// SRT를 위하여 회전 전에 원점으로 돌려놓는다.

	glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.f), glm::radians(degree), glm::vec3(0.f, 1.f, 0.f));
	modelTransform = rotateMatrix * modelTransform;

	setPosition(originPos);					// 기존 위치로 다시 이동
}

void GameObjectBase::setCollisionRadius(float radius)
{
	collisionRadius = radius;
}

float GameObjectBase::getCollisionRadius() const
{
	return collisionRadius;
}

bool GameObjectBase::isCollide(const GameObjectBase& other) const
{
	glm::vec3 aPos = getPosition();
	glm::vec3 bPos = other.getPosition();
	float collision_dist = collisionRadius + other.collisionRadius;

	float dist2 = (aPos.x - bPos.x) * (aPos.x - bPos.x) + (aPos.z - bPos.z) * (aPos.z - bPos.z);
	return dist2 <= collision_dist * collision_dist;
}

BoundingBox GameObjectBase::getBoundingBox() const
{
	const glm::vec3 myPos = getPosition();
	BoundingBox boundingBox;
	boundingBox.setBound(1.f, 0.1f, myPos.z + collisionRadius, myPos.z - collisionRadius, myPos.x - collisionRadius, myPos.x + collisionRadius);
	return boundingBox;
}

