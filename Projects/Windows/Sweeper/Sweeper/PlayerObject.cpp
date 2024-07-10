#include "PlayerObject.h"

#include "GLTFModelObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#define PLAYER_CLIP_IDLE			19
#define PLAYER_CLIP_RUN				24
#define PLAYER_CLIP_DASH			7

StateMachine::StateMachine(PlayerObject& player)
	: player{ player }
{
}

void StateMachine::enter()
{
}

void StateMachine::update(float elapsedTime, uint32_t currentFrame)
{
	// Ŭ��� ���� ���� ó���� ���� ���� �ʴ´�
}

void StateMachine::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void StateMachine::exit()
{
}

PLAYER_STATE StateMachine::getState() const
{
	return state;
}

IDLEState::IDLEState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::IDLE;
}

void IDLEState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_IDLE);
	player.setAnimateSpeed(1.f);
}

void IDLEState::update(float elapsedTime, uint32_t currentFrame)
{
	// IDLE�� ���¿��� �� ��..

	StateMachine::update(elapsedTime, currentFrame);
}

void IDLEState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void IDLEState::exit()
{
	StateMachine::exit();
}

RUNState::RUNState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::RUN;

	maxMoveSpeed = PLAYER_SPEED;				// �ʴ� �̵��ӵ� 5m
	moveSpeed = 0.f;
	acceleration = 25.f;

	lastDirection = glm::vec3{ 0.f };
}

void RUNState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_RUN);
	player.setAnimateSpeed(1.f);
}

void RUNState::update(float elapsedTime, uint32_t currentFrame)
{
	bool isKeyOn = player.keyState & KEY_UP or player.keyState & KEY_DOWN or
		player.keyState & KEY_LEFT or player.keyState & KEY_RIGHT;
	if (isKeyOn) {
		// ���ӵ� ����
		moveSpeed += acceleration * elapsedTime;
		if (moveSpeed > maxMoveSpeed)
			moveSpeed = maxMoveSpeed;

		glm::vec3 direction{ 0.f };
		if (player.keyState & KEY_UP) direction.z += 1.f;
		if (player.keyState & KEY_DOWN) direction.z -= 1.f;
		if (player.keyState & KEY_LEFT) direction.x += 1.f;
		if (player.keyState & KEY_RIGHT) direction.x -= 1.f;

		lastDirection = direction;
	}
	else {	// Ű�� ������ ��
		moveSpeed -= acceleration * elapsedTime;
		if (moveSpeed < 0.f) {
			moveSpeed = 0.f;
			// Ű�� ������ �ӵ��� 0�� �Ǹ� IDLE�� �ٲ�� (�������� �ٲ�)
		}
	}

	//move(direction, elapsedTime * moveSpeed);
	// �Էµ� �������� �÷��̾� ������ ���� �ٲٸ鼭, �ش� �������� �����Ѵ�
	// �̵��ϸ� �ʰ� �浹üũ�� �Ͽ� �� �� �ִ� ���� ����
	player.moveAndCheckCollision(lastDirection, moveSpeed, elapsedTime);

	// �÷��̾�� ���Ϳ��� �浹ó���� ����������

	StateMachine::update(elapsedTime, currentFrame);
}

void RUNState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void RUNState::exit()
{
	StateMachine::exit();
}

DASHState::DASHState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::DASH;

	direction = glm::vec3{ 0.f };
	if (player.keyState & KEY_UP) direction.z += 1.f;
	if (player.keyState & KEY_DOWN) direction.z -= 1.f;
	if (player.keyState & KEY_LEFT) direction.x += 1.f;
	if (player.keyState & KEY_RIGHT) direction.x -= 1.f;

	accFlag = 1;

	maxMoveSpeed = 30.f;
	moveSpeed = 0.f;
	acceleration = 250.f;
}

void DASHState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_DASH);
	player.setAnimateSpeed(2.5f);
}

void DASHState::update(float elapsedTime, uint32_t currentFrame)
{
	if (accFlag > 0) {
		moveSpeed += acceleration * elapsedTime;
		if (moveSpeed >= maxMoveSpeed) {
			moveSpeed = maxMoveSpeed;
			accFlag = -1;
		}
	}
	else {
		moveSpeed -= acceleration * elapsedTime;
		if (moveSpeed < 0.f)
			moveSpeed = 0.f;
	}
	player.moveAndCheckCollision(direction, moveSpeed, elapsedTime);

	StateMachine::update(elapsedTime, currentFrame);
}

void DASHState::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
}

void DASHState::exit()
{
	StateMachine::exit();
}

PlayerObject::PlayerObject(GLTFModelObject& mapObject)
	: mapObject{ mapObject }
{
	currentState = std::make_unique<IDLEState>(*this);

	collisionRadius = 0.4f;						// ĳ���� �浹 ������ ����
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime, uint32_t currentFrame)
{
	// Ű �Է¿� ���� �������� �������� ó��
	currentState->update(elapsedTime, currentFrame);

	GLTFSkinModelObject::update(elapsedTime, currentFrame);
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);

	currentState->draw(commandBuffer, pipelineLayout, currentFrame);
}

void PlayerObject::release()
{
}

void PlayerObject::processKeyInput(unsigned int key, bool is_pressed)
{
	if (is_pressed) {
		keyState |= key;
	}
	else {
		keyState &= ~key;
	}
}

void PlayerObject::changeIDLEState()
{
	currentState->exit();
	currentState = std::make_unique<IDLEState>(*this);
	currentState->enter();
}

void PlayerObject::changeRUNState()
{
	currentState->exit();
	currentState = std::make_unique<RUNState>(*this);
	currentState->enter();
}

void PlayerObject::changeDASHState()
{
	currentState->exit();
	currentState = std::make_unique<DASHState>(*this);
	currentState->enter();
}

void PlayerObject::rotateAndMoveToDirection(const glm::vec3& direction, float moveSpeed, float elapsedTime)
{
	if (glm::length(direction) <= 0.f)
		return;

	glm::vec3 look = getLook();
	glm::vec3 dir = glm::normalize(direction);
	glm::vec3 crossProduct = glm::cross(look, dir);
	//float dotProduct = glm::dot(look, dir);
	//float radianAngle = glm::acos(glm::clamp(dotProduct, -1.f, 1.f));				// 0 ~ pi (�������� ���´�)
	float radianAngle = glm::angle(look, dir);				// 0 ~ pi (�������� ���´�)

	float rotateSign = 1.f;
	if (crossProduct.y < 0.f)								// �ð�������� ���� �ݽð�������� ���� �����ش�
		rotateSign = -1.f;

	float angleOffset = radianAngle / glm::pi<float>();		// ������ ���� 0.f ~ 1.f
	float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// �⺻ ���� 0.f ~ 0.5f�̰�, �ּҰ��� 0.2f

	rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

	move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// ���� ȸ�� ���⿡ ���� �ӵ� ����
}

void PlayerObject::moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime)
{
	auto befPos = getPosition();
	rotateAndMoveToDirection(direction, moveSpeed, elapsedTime);
	auto& boundBox = mapObject.getBoundingBox();
	for (const auto& box : boundBox) {
		if (box.isCollide(getBoundingBox())) {
			setPosition(befPos);			// �浹�� ���� ���� ��ġ�� ���ư���.
			// �����̵� ���� ����
			glm::vec3 pDir = getLook();	// ���� �����ִ� ����
			pDir.y = 0.f;
			glm::vec3 moveDir = pDir;			// �̵��� ����
			auto pBox = getBoundingBox();// �浹 ���� �ٿ�� �ڽ�
			// ���� �������� ��� ������ �浹�̾����� Ȯ�� (�浹 �� ��ġ�� ���ư�����, �ش� ���⿡���� ���� �浹�� �ƴϴ�)
			glm::vec3 newPos = befPos;
			if (box.getBack() > pBox.getFront() or box.getFront() < pBox.getBack()) {
				moveDir.z = 0.f;
				if (box.getBack() > pBox.getFront()) {	// �÷��̾ �ڽ��� ����
					newPos.z = box.getBack() - getCollisionRadius() - 0.00001f;
				}
				else {									// �÷��̾ �ڽ��� ����
					newPos.z = box.getFront() + getCollisionRadius() + 0.00001f;
				}
			}
			if (box.getLeft() > pBox.getRight() or box.getRight() < pBox.getLeft()) {
				moveDir.x = 0.f;
				if (box.getLeft() > pBox.getRight()) {	// �÷��̾ �ڽ��� ����
					newPos.x = box.getLeft() - getCollisionRadius() - 0.00001f;
				}
				else {									// �÷��̾ �ڽ��� ������
					newPos.x = box.getRight() + getCollisionRadius() + 0.00001f;
				}
			}
			setPosition(newPos);			// �浹�� �ƴ� �ִ��� ��ġ�� ������ �ش�.
			if (glm::length(moveDir) > 0.f) {
				// �������� �ι踦 �Ͽ� ������ ���� �����ϰ� �ι��� �ӵ�, �ִ� 1.f�� �ӵ��� �����̵��� �Ѵ�.
				float moveOffset = glm::clamp(glm::dot(pDir, glm::normalize(moveDir)) * 2.f, 0.f, 1.f);
				move(moveDir, moveOffset * moveSpeed * elapsedTime);
				// �����̵��� �Ͽ������� �浹�̸� ���� ��ġ�� ���ư���, ���̻� �������� �ʴ´�.
				for (const auto& box2 : boundBox) {
					if (box2.isCollide(getBoundingBox())) {
						setPosition(newPos);
						break;
					}
				}
			}
			break;
		}
	}
}
