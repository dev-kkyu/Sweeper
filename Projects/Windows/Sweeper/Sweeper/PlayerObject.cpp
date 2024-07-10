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
	// 클라는 현재 점프 처리를 따로 하지 않는다
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
	// IDLE인 상태에서 할 일..

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

	maxMoveSpeed = PLAYER_SPEED;				// 초당 이동속도 5m
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
		// 가속도 적용
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
	else {	// 키가 떼졌을 때
		moveSpeed -= acceleration * elapsedTime;
		if (moveSpeed < 0.f) {
			moveSpeed = 0.f;
			// 키가 떼지고 속도가 0이 되면 IDLE로 바뀐다 (서버에서 바뀜)
		}
	}

	//move(direction, elapsedTime * moveSpeed);
	// 입력된 방향으로 플레이어 방향을 점차 바꾸면서, 해당 방향으로 전진한다
	// 이동하며 맵과 충돌체크를 하여 갈 수 있는 곳만 간다
	player.moveAndCheckCollision(lastDirection, moveSpeed, elapsedTime);

	// 플레이어와 몬스터와의 충돌처리는 서버에서만

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

	collisionRadius = 0.4f;						// 캐릭터 충돌 반지름 조정
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime, uint32_t currentFrame)
{
	// 키 입력에 따른 움직임은 서버에서 처리
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
	//float radianAngle = glm::acos(glm::clamp(dotProduct, -1.f, 1.f));				// 0 ~ pi (예각으로 나온다)
	float radianAngle = glm::angle(look, dir);				// 0 ~ pi (예각으로 나온다)

	float rotateSign = 1.f;
	if (crossProduct.y < 0.f)								// 시계방향으로 돌지 반시계방향으로 돌지 정해준다
		rotateSign = -1.f;

	float angleOffset = radianAngle / glm::pi<float>();		// 각도에 따라 0.f ~ 1.f
	float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// 기본 값은 0.f ~ 0.5f이고, 최소값은 0.2f

	rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

	move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// 현재 회전 방향에 따른 속도 조절
}

void PlayerObject::moveAndCheckCollision(const glm::vec3& direction, float moveSpeed, float elapsedTime)
{
	auto befPos = getPosition();
	rotateAndMoveToDirection(direction, moveSpeed, elapsedTime);
	auto& boundBox = mapObject.getBoundingBox();
	for (const auto& box : boundBox) {
		if (box.isCollide(getBoundingBox())) {
			setPosition(befPos);			// 충돌시 먼저 기존 위치로 돌아간다.
			// 슬라이딩 벡터 구현
			glm::vec3 pDir = getLook();	// 현재 보고있는 방향
			pDir.y = 0.f;
			glm::vec3 moveDir = pDir;			// 이동할 방향
			auto pBox = getBoundingBox();// 충돌 전의 바운딩 박스
			// 기존 지점에서 어느 방향이 충돌이었는지 확인 (충돌 전 위치로 돌아갔으니, 해당 방향에서는 현재 충돌이 아니다)
			glm::vec3 newPos = befPos;
			if (box.getBack() > pBox.getFront() or box.getFront() < pBox.getBack()) {
				moveDir.z = 0.f;
				if (box.getBack() > pBox.getFront()) {	// 플레이어가 박스의 뒤쪽
					newPos.z = box.getBack() - getCollisionRadius() - 0.00001f;
				}
				else {									// 플레이어가 박스의 앞쪽
					newPos.z = box.getFront() + getCollisionRadius() + 0.00001f;
				}
			}
			if (box.getLeft() > pBox.getRight() or box.getRight() < pBox.getLeft()) {
				moveDir.x = 0.f;
				if (box.getLeft() > pBox.getRight()) {	// 플레이어가 박스의 왼쪽
					newPos.x = box.getLeft() - getCollisionRadius() - 0.00001f;
				}
				else {									// 플레이어가 박스의 오른쪽
					newPos.x = box.getRight() + getCollisionRadius() + 0.00001f;
				}
			}
			setPosition(newPos);			// 충돌이 아닌 최대의 위치로 움직여 준다.
			if (glm::length(moveDir) > 0.f) {
				// 내적값의 두배를 하여 직각일 때를 제외하고 두배의 속도, 최대 1.f의 속도로 움직이도록 한다.
				float moveOffset = glm::clamp(glm::dot(pDir, glm::normalize(moveDir)) * 2.f, 0.f, 1.f);
				move(moveDir, moveOffset * moveSpeed * elapsedTime);
				// 슬라이딩을 하였음에도 충돌이면 기존 위치로 돌아가고, 더이상 움직이지 않는다.
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
