#include "PlayerObject.h"

#include "NetworkManager.h"		// KEY EVENT Define을 가져오기 위함

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

PlayerObject::PlayerObject()
{
	maxMoveSpeed = PLAYER_SPEED;				// 초당 이동속도 5m
	moveSpeed = 0.f;
	acceleration = 25.f;
	lastDirection = glm::vec3{};

	dashFlag = true;
	dashAccFlag = 1;							// Todo : 이 변수들은 서버와 동기화 필요
	dashMaxMoveSpeed = 30.f;
	dashMoveSpeed = 0.f;
	dashAcceleration = 250.f;

	collisionRadius = 0.4f;						// 캐릭터 충돌 반지름 조정
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::initialize()
{
}

void PlayerObject::update(float elapsedTime, uint32_t currentFrame)
{
	// 키 입력에 따른 움직임은 서버에서 처리

	GLTFSkinModelObject::update(elapsedTime, currentFrame);

	// 클라이언트에서 플레이어 이동 보정
	if (keyState & MOUSE_LEFT or (dashFlag and keyState & KEY_CTRL)) {
		moveSpeed = 0.f;
		if (keyState & KEY_CTRL) {
			dashMoveSpeed = 0.f;
			dashAccFlag = 1;
			dashFlag = false;	// Ctrl이 처음 눌렸을 때 초기화
		}
	}
	else if (PLAYER_CLIP_DASH == activeAnimation) {		// Dash 이동 보정
		if (dashAccFlag > 0) {
			dashMoveSpeed += dashAcceleration * elapsedTime;
			if (dashMoveSpeed >= dashMaxMoveSpeed) {
				dashMoveSpeed = dashMaxMoveSpeed;
				dashAccFlag = -1;
			}
		}
		else {
			dashMoveSpeed -= dashAcceleration * elapsedTime;
			if (dashMoveSpeed < 0.f)
				dashMoveSpeed = 0.f;
		}
		if (glm::length(lastDirection) >= glm::epsilon<float>()) {
			glm::vec3 look = getLook();
			glm::vec3 dir = glm::normalize(lastDirection);
			glm::vec3 crossProduct = glm::cross(look, dir);
			float radianAngle = glm::angle(look, dir);				// 0 ~ pi (예각으로 나온다)

			float rotateSign = 1.f;
			if (crossProduct.y < 0.f)								// 시계방향으로 돌지 반시계방향으로 돌지 정해준다
				rotateSign = -1.f;

			float angleOffset = radianAngle / glm::pi<float>();		// 각도에 따라 0.f ~ 1.f
			float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// 기본 값은 0.f ~ 0.5f이고, 최소값은 0.2f

			rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

			move(dir, elapsedTime * dashMoveSpeed * (1.f - angleOffset));	// 현재 회전 방향에 따른 속도 조절
		}
	}
	else {
		if (PLAYER_CLIP_RUN == activeAnimation) {	// RUN 이동 보정
			bool isKeyOn = keyState & KEY_UP or keyState & KEY_DOWN or
				keyState & KEY_LEFT or keyState & KEY_RIGHT;
			if (isKeyOn) {
				// 가속도 적용
				moveSpeed += acceleration * elapsedTime;
				if (moveSpeed > maxMoveSpeed)
					moveSpeed = maxMoveSpeed;

				glm::vec3 direction{ 0.f };
				if (keyState & KEY_UP) direction.z += 1.f;
				if (keyState & KEY_DOWN) direction.z -= 1.f;
				if (keyState & KEY_LEFT) direction.x += 1.f;
				if (keyState & KEY_RIGHT) direction.x -= 1.f;

				lastDirection = direction;
			}
			else {	// 키가 떼졌을 때
				moveSpeed -= acceleration * elapsedTime;
				if (moveSpeed < 0.f) {
					moveSpeed = 0.f;
				}
			}

			if (glm::length(lastDirection) >= glm::epsilon<float>()) {
				glm::vec3 look = getLook();
				glm::vec3 dir = glm::normalize(lastDirection);
				glm::vec3 crossProduct = glm::cross(look, dir);
				float radianAngle = glm::angle(look, dir);				// 0 ~ pi (예각으로 나온다)

				float rotateSign = 1.f;
				if (crossProduct.y < 0.f)								// 시계방향으로 돌지 반시계방향으로 돌지 정해준다
					rotateSign = -1.f;

				float angleOffset = radianAngle / glm::pi<float>();		// 각도에 따라 0.f ~ 1.f
				float rotateSpeed = glm::max(angleOffset / 2.f, 0.2f) * elapsedTime * 30.f;		// 기본 값은 0.f ~ 0.5f이고, 최소값은 0.2f

				rotate(glm::degrees(radianAngle * rotateSign) * rotateSpeed);

				move(dir, elapsedTime * moveSpeed * (1.f - angleOffset));	// 현재 회전 방향에 따른 속도 조절
			}
		}
		// Dash Animation이 아닐 땐 true
		dashFlag = true;
	}
}

void PlayerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
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

float PlayerObject::getMoveSpeed() const
{
	return moveSpeed;
}
