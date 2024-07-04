#pragma once

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class PlayerObject;
class Camera
{
private:
	int& winWidth;
	int& winHeight;

	float distanceFromPlayer;	// ������ ȣ�� �� �� �����ֱ�

	glm::mat4 viewTransform{ 1.f };
	glm::mat4 projectionTransform{ 1.f };

	std::shared_ptr<PlayerObject> pPlayer;

	// ī�޶� ȸ���� ���� ���콺 ���� ��ǥ (-1 ~ 1)
	float startXpos = 0.f;
	float startYpos = 0.f;

	// ī�޶� ���Ʒ��� ȸ�� ����
	float pitchAngle;			// ������ ȣ�� �� �� �����ֱ�

public:
	Camera(int& width, int& height);
	~Camera();

	void update(float elapsedTime);

	void setPlayer(std::shared_ptr<PlayerObject> pPlayer);

	glm::mat4 getView() const;
	glm::mat4 getProjection() const;

	void setDistance(float distance);
	float getDistance() const;

	void setStartMousePos(float xpos, float ypos);
	void processMouseCursor(float xpos, float ypos);

};
