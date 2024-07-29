#pragma once

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct VkExtent2D;
class PlayerObject;
class Camera
{
private:
	VkExtent2D& framebufferExtent;

	float distanceFromPlayer;	// 생성자 호출 시 값 정해주기

	glm::mat4 viewTransform{ 1.f };
	glm::mat4 projectionTransform{ 1.f };

	std::shared_ptr<PlayerObject> pPlayer;

	// 카메라 회전을 위한 마우스 시작 좌표 (-1 ~ 1)
	float startXpos = 0.f;
	float startYpos = 0.f;

	// 카메라 위아래의 회전 각도
	float pitchAngle;			// 생성자 호출 시 값 정해주기

public:
	Camera(VkExtent2D& framebufferExtent);
	~Camera();

	void update(float elapsedTime);

	void setPlayer(std::shared_ptr<PlayerObject> pPlayer);
	const std::shared_ptr<PlayerObject>& getPlayer() const;

	glm::mat4 getView() const;
	glm::mat4 getProjection() const;

	void setDistance(float distance);
	float getDistance() const;
	void setPitchAngle(float angle);

	void setStartMousePos(float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

};
