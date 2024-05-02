#include "Camera.h"

#include "PlayerObject.h"

#include <glm/gtx/vector_angle.hpp>

Camera::Camera()
	: distanceFromPlayer{ 10.f }		// ī�޶� �Ÿ�
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

		glm::vec3 center = position;							// ī�޶�� �÷��̾ �ٶ󺻴�.
		center.y += 2.2f;										// �÷��̾��� ������ �ٴ��̹Ƿ� �÷��ش�.

		// ������ look(0, 0, 1)�� ���ؼ�, �󸶳� ȸ���Ǿ����� ���
		float xzAngle = glm::orientedAngle(glm::vec3(0.f, 0.f, 1.f), look, glm::vec3(0.f, 1.f, 0.f));
		// ������ cameraPos(0, 0, -distance)�� x�࿡ ���Ͽ� ȸ��(ī�޶� ���� ����) (���� ī�޶�� ȭ�� ���ʿ� �ִ�)
		glm::vec4 cameraPos = glm::rotate(glm::mat4(1.f), glm::radians(pitchAngle), glm::vec3(1.f, 0.f, 0.f)) * glm::vec4(0.f, 0.f, -distanceFromPlayer, 1.f);
		// ���������� y�࿡ ���Ͽ� ȸ�� (look �������� ����)
		cameraPos = glm::rotate(glm::mat4(1.f), xzAngle, glm::vec3(0.f, 1.f, 0.f)) * cameraPos;
		// ī�޶� ��ġ�� ���� �÷��̾� ��ġ�� �ű��, ī�޶� ��ȯ ����� ���
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
