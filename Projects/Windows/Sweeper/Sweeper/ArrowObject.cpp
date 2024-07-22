#include "ArrowObject.h"

#include <algorithm>

ArrowObject::ArrowObject(vkf::Effect& effect)
	: effect{ effect }
{
	spawnTime = std::chrono::steady_clock::now();
}

void ArrowObject::initialize()
{
}

void ArrowObject::update(float elapsedTime, uint32_t currentFrame)
{
	if (spawnTime + std::chrono::milliseconds(375) <= std::chrono::steady_clock::now()) {
		moveForward(20.f * elapsedTime);		// 화살 속도는 서버와 동기화 해줘야 함
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

void ArrowObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	auto nowTime = std::chrono::steady_clock::now();
	if (spawnTime + std::chrono::milliseconds(375) <= nowTime) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);
		glm::mat4 matrix = glm::translate(glm::mat4(1.f), getPosition() + getLook());
		matrix[3][3] = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - spawnTime).count() / 1000.f;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
}
