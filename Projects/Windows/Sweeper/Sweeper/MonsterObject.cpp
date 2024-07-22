#include "MonsterObject.h"

MonsterObject::MonsterObject()
{
}

MonsterObject::~MonsterObject()
{
}

void MonsterObject::initialize()
{
}

void MonsterObject::update(float elapsedTime, uint32_t currentFrame)
{
	GLTFSkinModelObject::update(elapsedTime, currentFrame);

	// 이동 보정
	if (MONSTER_CLIP_MOVE == activeAnimation) {
		moveForward(2.f * elapsedTime);		// Todo : 서버와 공유 변수 필요 (이동 속도)
	}
}

void MonsterObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	GLTFSkinModelObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void MonsterObject::release()
{
}

void MonsterObject::drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	glm::mat4 matrix = glm::scale(glm::mat4(1.f), glm::vec3{ 0.5f, 0.05f, 1.f });
	auto pos = getPosition();
	matrix = glm::translate(glm::mat4(1.f), glm::vec3(pos.x, pos.y + headHeight, pos.z)) * matrix;
	matrix[3][3] = float(HP) / float(maxHP);
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
	vkCmdDraw(commandBuffer, 12, 1, 0, 0);
}

void MonsterObject::setHeadHeight(float height)
{
	headHeight = height;
}
