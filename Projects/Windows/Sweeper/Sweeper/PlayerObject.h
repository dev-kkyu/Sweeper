#pragma once

#include "GLTFSkinModelObject.h"

#include "NetworkManager.h"		// KEY EVENT Define, Player State�� �������� ����

#include <chrono>

class PlayerObject;
class StateMachine
{
protected:
	PLAYER_STATE state;
	PlayerObject& player;

public:
	StateMachine(PlayerObject& player);
	virtual ~StateMachine() = default;

	virtual void enter() = 0;
	virtual void update(float elapsedTime, uint32_t currentFrame) = 0;
	virtual void exit() = 0;
	virtual PLAYER_STATE getState() const final;
};

class IDLEState : public StateMachine
{
public:
	IDLEState(PlayerObject& player);
	virtual ~IDLEState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class RUNState : public StateMachine
{
private:
	// �����̴� �ӵ�	// ���� �����ڿ���
	float maxMoveSpeed;		// �ִ� �ӵ�
	float moveSpeed;		// ���� �ӵ�
	float acceleration;		// ���� ���ӵ�
	glm::vec3 lastDirection;	// �����̴� ���� (Ű�� ���� �� �� ����, xz�� ���)

public:
	RUNState(PlayerObject& player);
	virtual ~RUNState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class DASHState : public StateMachine
{
private:
	int accFlag;			// ���� �÷��� (����, ����)

	// �����̴� �ӵ�	// ���� �����ڿ���
	float maxMoveSpeed;		// �ִ� �ӵ�
	float moveSpeed;		// ���� �ӵ�
	float acceleration;		// ���� ���ӵ�
	glm::vec3 direction;	// �̵��� ���� (xz�� ���)

public:
	DASHState(PlayerObject& player);
	virtual ~DASHState() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void exit() override;
};

class GLTFModelObject;
class PlayerObject : public GLTFSkinModelObject
{
	friend class StateMachine;
	friend class IDLEState;
	friend class RUNState;
	friend class DASHState;
	friend class WarriorAttackState;
	friend class WarriorSKILLState;
	friend class ArchorAttackState;
	friend class ArchorSKILLState;
	friend class MageAttackState;
	friend class MageSKILLState;
	friend class HealerAttackState;
	friend class HealerSKILLState;

protected:
	// �ʰ��� �浹ó���� ���� MapObject
	GLTFModelObject& mapObject;

	// ���� ����
	std::unique_ptr<StateMachine> currentState;

	// Ű�� ������ ���¸� �����Ѵ�.
	unsigned int keyState = 0;

public:
	PlayerObject(GLTFModelObject& mapObject);
	virtual ~PlayerObject() = default;

	virtual void initialize() override;
	virtual void update(float elapsedTime, uint32_t currentFrame) override;
	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) override;
	virtual void release() override;

	virtual void drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) = 0;

	void processKeyInput(unsigned int key, bool is_pressed);

	virtual void changeIDLEState() final;
	virtual void changeRUNState() final;
	virtual void changeDASHState() final;
	virtual void changeATTACKState() = 0;
	virtual void changeSKILLState() = 0;

protected:
	void rotateAndMoveToDirection(const glm::vec3& direction, float moveSpeed, float elapsedTime);
	void moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime);

};
