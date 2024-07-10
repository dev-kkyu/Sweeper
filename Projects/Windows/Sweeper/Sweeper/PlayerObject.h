#pragma once

#include "GLTFSkinModelObject.h"

#include "NetworkManager.h"		// KEY EVENT Define, Player State를 가져오기 위함

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
	// 움직이는 속도	// 값은 생성자에서
	float maxMoveSpeed;		// 최대 속도
	float moveSpeed;		// 현재 속도
	float acceleration;		// 현재 가속도
	glm::vec3 lastDirection;	// 움직이던 방향 (키를 뗐을 때 갈 방향, xz만 사용)

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
	int accFlag;			// 가속 플래그 (증가, 감소)

	// 움직이는 속도	// 값은 생성자에서
	float maxMoveSpeed;		// 최대 속도
	float moveSpeed;		// 현재 속도
	float acceleration;		// 현재 가속도
	glm::vec3 direction;	// 이동할 방향 (xz만 사용)

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
	// 맵과의 충돌처리를 위한 MapObject
	GLTFModelObject& mapObject;

	// 상태 관리
	std::unique_ptr<StateMachine> currentState;

	// 키가 눌려진 상태를 종합한다.
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
