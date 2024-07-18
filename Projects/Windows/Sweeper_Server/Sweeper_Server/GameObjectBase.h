#pragma once

#include "BoundingBox.h"

#include <memory>

class Room;
class GameObjectBase : public std::enable_shared_from_this<GameObjectBase>	// ������Ʈ ��ü������ Ÿ�̸� ������ shared_ptr �ʿ�
{
protected:
	// ������Ʈ �Ҽ� ����, �÷��̾�� ���Ǿȿ�, ���ʹ� monsters
	Room* parentRoom;
	int my_id;			// �÷��̾� ID Ȥ�� ����ID

protected:
	// up�� �׻� (0.f, 1.f, 0.f)�̶�� �����Ѵ�.
	glm::mat4 modelTransform{ 1.f };
	glm::vec3 scaleValue{ 1.f };

	// �浹 ������ (�⺻ 0.5f)
	float collisionRadius = 0.5f;

public:
	GameObjectBase(Room* parentRoom, int o_id);
	virtual ~GameObjectBase();

	virtual void initialize() = 0;
	virtual bool update(float elapsedTime) = 0;		// ������Ʈ �� ���� ������ false
	virtual void release() = 0;
	virtual void onHit(const GameObjectBase& other, int damage) = 0;

	virtual void setPosition(glm::vec3 position) final;
	virtual void setLook(glm::vec3 look) final;					// y�� �׻� 0���� ����
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
