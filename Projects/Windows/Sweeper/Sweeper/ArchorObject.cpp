#include "ArchorObject.h"

#include <algorithm>

#define PLAYER_CLIP_ATTACK_ARCHOR	26
#define PLAYER_CLIP_SKILL_ARCHOR	26

ArchorATTACKState::ArchorATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArchorATTACKState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_ARCHOR);
	player.setAnimateSpeed(0.8f);
}

void ArchorATTACKState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArchorATTACKState::exit()
{
	StateMachine::exit();
}

ArchorSKILLState::ArchorSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArchorSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_ARCHOR);
	player.setAnimateSpeed(0.8f);

	dynamic_cast<ArchorObject*>(&player)->archorEffects.push_back(ArchorObject::ArchorEffect{ player.getPosition(), player.getLook(), -0.375f });
}

void ArchorSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArchorSKILLState::exit()
{
	StateMachine::exit();
}

ArchorObject::ArchorObject(GLTFModelObject& mapObject, vkf::Effect& effect)
	: PlayerObject{ mapObject }, effect{ effect }
{
}

void ArchorObject::initialize()
{
}

void ArchorObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& aEffect : archorEffects) {
		aEffect.accumTime += elapsedTime;
		if (aEffect.accumTime > 0.f)
			aEffect.pos += aEffect.dir * 12.f * elapsedTime;
	}

	std::list<std::vector<ArchorObject::ArchorEffect>::iterator> deleteEffects;
	for (auto itr = archorEffects.begin(); itr != archorEffects.end(); ++itr) {
		if (itr->accumTime >= 0.625f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		archorEffects.erase(itr);
	}
}

void ArchorObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void ArchorObject::release()
{
}

void ArchorObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (archorEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

		// 투명한 물체 정렬
		std::vector<ArchorObject::ArchorEffect> sortEffects = archorEffects;
		std::sort(sortEffects.begin(), sortEffects.end(),
			[](const ArchorObject::ArchorEffect& a, const ArchorObject::ArchorEffect& b) {
				return a.pos.z > b.pos.z;
			});

		for (auto& aEffect : sortEffects) {
			if (aEffect.accumTime >= 0.f) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.f), aEffect.pos);
				matrix[3][3] = aEffect.accumTime;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			}
		}
	}
}

void ArchorObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<ArchorATTACKState>(*this);
	currentState->enter();
}

void ArchorObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<ArchorSKILLState>(*this);
	currentState->enter();
}
