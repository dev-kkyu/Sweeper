#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class PlayerObject;
class Camera
{
private:
	glm::mat4 viewTransform{ 1.f };
	glm::mat4 projectionTransform{ 1.f };

	PlayerObject* pPlayer = nullptr;

public:
	Camera();
	~Camera();

	void update(float elapsedTime);

	void setPlayer(PlayerObject* pPlayer);

	glm::mat4 getView() const;
	glm::mat4 getProjection() const;

};
