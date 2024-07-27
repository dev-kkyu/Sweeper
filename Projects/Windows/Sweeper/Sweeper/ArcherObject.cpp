#include "ArcherObject.h"

#define PLAYER_CLIP_ATTACK_ARCHER	26
#define PLAYER_CLIP_SKILL_ARCHER	26

ArcherATTACKState::ArcherATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void ArcherATTACKState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_ARCHER);
	player.setAnimateSpeed(0.8f);
}

void ArcherATTACKState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArcherATTACKState::exit()
{
	StateMachine::exit();
}

ArcherSKILLState::ArcherSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void ArcherSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_ARCHER);
	player.setAnimateSpeed(0.8f);

	dynamic_cast<ArcherObject*>(&player)->archerEffects.push_back(ArcherObject::ArcherEffect{ player.getPosition(), player.getLook(), -0.375f });
}

void ArcherSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void ArcherSKILLState::exit()
{
	StateMachine::exit();
}

ArcherObject::ArcherObject(GLTFModelObject& mapObject, vkf::Effect& effect)
	: PlayerObject{ mapObject }, effect{ effect }
{
	player_type = PLAYER_TYPE::ARCHER;

	maxHP = HP = MAX_HP_PLAYER_ARCHER;
}

void ArcherObject::initialize()
{
}

void ArcherObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& aEffect : archerEffects) {
		aEffect.accumTime += elapsedTime;
		if (aEffect.accumTime > 0.f)
			aEffect.pos += aEffect.dir * 12.f * elapsedTime;
	}

	std::list<std::list<ArcherObject::ArcherEffect>::iterator> deleteEffects;
	for (auto itr = archerEffects.begin(); itr != archerEffects.end(); ++itr) {
		if (itr->accumTime >= 0.625f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		archerEffects.erase(itr);
	}
}

void ArcherObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void ArcherObject::release()
{
}

void ArcherObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	if (archerEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

		// 투명한 물체 정렬
		std::list<ArcherObject::ArcherEffect> sortEffects = archerEffects;
		sortEffects.sort(
			[](const ArcherObject::ArcherEffect& a, const ArcherObject::ArcherEffect& b) {
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

void ArcherObject::drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	GLTFSkinModelObject::drawUI(commandBuffer, pipelineLayout);
}

void ArcherObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<ArcherATTACKState>(*this);
	currentState->enter();
}

void ArcherObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<ArcherSKILLState>(*this);
	currentState->enter();
}
