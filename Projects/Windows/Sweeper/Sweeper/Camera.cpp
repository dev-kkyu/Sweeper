#include "Camera.h"

#include "PlayerObject.h"

Camera::Camera()
{
	viewTransform = glm::lookAt(glm::vec3(0.f, 3.f, 10.f), glm::vec3(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
    projectionTransform = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 0.1f, 100.0f);
}

Camera::~Camera()
{
}

void Camera::update(float elapsedTime)
{
    if (pPlayer) {
        glm::vec3 position = pPlayer->getPosition();
        glm::vec3 look = pPlayer->getLook();

        glm::vec3 eye = position - (look * 4.f);
        eye.y += 2.5f;

        glm::vec3 center = position;
        center.y += 2.f;

		viewTransform = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
        //projectionTransform = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 0.1f, 10.0f);
    }
}

void Camera::setPlayer(PlayerObject* pPlayer)
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
