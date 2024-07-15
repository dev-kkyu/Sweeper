#include "ArrowObject.h"

ArrowObject::ArrowObject()
{
	spawnTime = std::chrono::steady_clock::now();
}

void ArrowObject::initialize()
{
}

void ArrowObject::update(float elapsedTime, uint32_t currentFrame)
{
	if (spawnTime + std::chrono::milliseconds(375) <= std::chrono::steady_clock::now()) {
		moveForward(15.f * elapsedTime);		// 화살 속도는 서버와 동기화 해줘야 함
		updateBoundingBox();
	}
}

void ArrowObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (spawnTime + std::chrono::milliseconds(375) <= std::chrono::steady_clock::now()) {
		GLTFModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
	}
}

void ArrowObject::release()
{
}
