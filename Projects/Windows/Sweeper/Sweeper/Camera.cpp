#include "Camera.h"

#include "PlayerObject.h"

#include <glm/gtx/vector_angle.hpp>

Camera::Camera(int& width, int& height)
	: winWidth{ width }, winHeight{ height }
	, distanceFromPlayer{ 14.f }		// 카메라 거리
	, pitchAngle{ 45.f }					// 카메라 각도
{
	viewTransform = glm::lookAt(glm::vec3(0.f, 3.f, 10.f), glm::vec3(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
	projectionTransform = glm::perspective(glm::radians(45.0f), float(winWidth) / float(winHeight), 1.f, 100.0f);
}

Camera::~Camera()
{
}

void Camera::update(float elapsedTime)
{
	if (pPlayer) {
		glm::vec3 position = pPlayer->getPosition();

		// 기존의 cameraPos(0, 0, -distance)를 x축에 대하여 회전(카메라 높이 조절) (기존 카메라는 화면 안쪽에 있다)
		glm::vec4 cameraPos = glm::rotate(glm::mat4(1.f), glm::radians(pitchAngle), glm::vec3(1.f, 0.f, 0.f)) * glm::vec4(0.f, 0.f, -distanceFromPlayer, 1.f);
		// 카메라 위치를 실제 플레이어 위치로 옮기고, 카메라 변환 행렬을 계산
		viewTransform = glm::lookAt(glm::vec3(cameraPos) + position, position, glm::vec3(0.0f, 1.0f, 0.0f));
		projectionTransform = glm::perspective(glm::radians(45.0f), float(winWidth) / float(winHeight), 1.f, 100.0f);
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

void Camera::setDistance(float distance)
{
	distanceFromPlayer = distance;
}

float Camera::getDistance() const
{
	return distanceFromPlayer;
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
	pitchAngle = glm::clamp(pitchAngle, -30.f, 89.99f);
}
