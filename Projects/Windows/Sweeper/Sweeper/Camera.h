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
	glm::mat4 viewTransform{ 1.f };
	glm::mat4 projectionTransform{ 1.f };

	std::shared_ptr<PlayerObject> pPlayer;

public:
	Camera();
	~Camera();

	void update(float elapsedTime);

	void setPlayer(std::shared_ptr<PlayerObject> pPlayer);

	glm::mat4 getView() const;
	glm::mat4 getProjection() const;

};
