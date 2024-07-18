#pragma once

#include "BoundingBox.h"

#include <memory>

class Room;
class GameObjectBase : public std::enable_shared_from_this<GameObjectBase>	// 오브젝트 자체적으로 타이머 쓰려면 shared_ptr 필요
{
protected:
	// 오브젝트 소속 정보, 플레이어는 세션안에, 몬스터는 monsters
	Room* parentRoom;
	int my_id;			// 플레이어 ID 혹은 몬스터ID

protected:
	// up은 항상 (0.f, 1.f, 0.f)이라고 가정한다.
	glm::mat4 modelTransform{ 1.f };
	glm::vec3 scaleValue{ 1.f };

	// 충돌 반지름 (기본 0.5f)
	float collisionRadius = 0.5f;

public:
	GameObjectBase(Room* parentRoom, int o_id);
	virtual ~GameObjectBase();

	virtual void initialize() = 0;
	virtual bool update(float elapsedTime) = 0;		// 업데이트 된 것이 없으면 false
	virtual void release() = 0;
	virtual void onHit(const GameObjectBase& other, int damage) = 0;

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;					// y는 항상 0으로 가정
	virtual void setScale(glm::vec3 scale) final;
	virtual glm::vec3 getPosition() const final;
	virtual glm::vec3 getLook() const final;
	virtual glm::vec3 getRight() const final;

	virtual void moveForward(float value) final;
	virtual void move(glm::vec3 direction, float value) final;

	virtual void rotate(float degree) final;

	virtual void setCollisionRadius(float radius) final;
	virtual float getCollisionRadius() const final;
	virtual bool isCollide(const GameObjectBase& other) const final;
	virtual BoundingBox getBoundingBox() const final;

};
