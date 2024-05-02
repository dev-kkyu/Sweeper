#include "Camera.h"

#include "PlayerObject.h"

#include <glm/gtx/vector_angle.hpp>

Camera::Camera()
	: distanceFromPlayer{ 10.f }		// 카메라 거리
{
	viewTransform = glm::lookAt(glm::vec3(0.f, 3.f, 10.f), glm::vec3(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
	projectionTransform = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 1.f, 500.0f);
}

Camera::~Camera()
{
}

void Camera::update(float elapsedTime)
{
	if (pPlayer) {
		glm::vec3 position = pPlayer->getPosition();
		glm::vec3 look = pPlayer->getLook();

		glm::vec3 center = position;							// 카메라는 플레이어를 바라본다.
		center.y += 2.2f;										// 플레이어의 원점이 바닥이므로 올려준다.

		// 기존의 look(0, 0, 1)과 비교해서, 얼마나 회전되었는지 계산
		float xzAngle = glm::orientedAngle(glm::vec3(0.f, 0.f, 1.f), look, glm::vec3(0.f, 1.f, 0.f));
		// 기존의 cameraPos(0, 0, -distance)를 x축에 대하여 회전(카메라 높이 조절) (기존 카메라는 화면 안쪽에 있다)
		glm::vec4 cameraPos = glm::rotate(glm::mat4(1.f), glm::radians(pitchAngle), glm::vec3(1.f, 0.f, 0.f)) * glm::vec4(0.f, 0.f, -distanceFromPlayer, 1.f);
		// 최종적으로 y축에 대하여 회전 (look 방향으로 조절)
		cameraPos = glm::rotate(glm::mat4(1.f), xzAngle, glm::vec3(0.f, 1.f, 0.f)) * cameraPos;
		// 카메라 위치를 실제 플레이어 위치로 옮기고, 카메라 변환 행렬을 계산
		viewTransform = glm::lookAt(glm::vec3(cameraPos) + center, center, glm::vec3(0.0f, 1.0f, 0.0f));
		//projectionTransform = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 0.1f, 10.0f);
	}
}

void Camera::setPlayer(std::shared_ptr<PlayerObject> pPlayer)
{
	this->pPlayer = pPlayer;
}

glm::mat4 Camera::getView() const
{
	return viewTransform;
}

glm::mat4 Camera::getProjection() const
{
	return projectionTransform;
}

void Camera::setStartMousePos(float xpos, float ypos)
{
	startXpos = xpos;
	startYpos = ypos;
}

void Camera::processMouseCursor(float xpos, float ypos)
{
	float moveX = xpos - startXpos;
	float moveY = ypos - startYpos;
	startXpos = xpos;
	startYpos = ypos;

	pitchAngle -= moveY * 40.f;
	pitchAngle = glm::clamp(pitchAngle, -15.f, 35.f);
}
