#include "HealerObject.h"

#define PLAYER_CLIP_ATTACK_HEALER	16
#define PLAYER_CLIP_SKILL_HEALER	31

HealerATTACKState::HealerATTACKState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::ATTACK;
}

void HealerATTACKState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_ATTACK_HEALER);
	player.setAnimateSpeed(0.4f);
}

void HealerATTACKState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void HealerATTACKState::exit()
{
	StateMachine::exit();
}

HealerSKILLState::HealerSKILLState(PlayerObject& player)
	: StateMachine{ player }
{
	state = PLAYER_STATE::SKILL;
}

void HealerSKILLState::enter()
{
	StateMachine::enter();

	player.setAnimationClip(PLAYER_CLIP_SKILL_HEALER);
	player.setAnimateSpeed(1.f);

	dynamic_cast<HealerObject*>(&player)->healerEffects.push_back(HealerObject::HealerEffect{ player.getPosition() + player.getLook() * 5.5f, -0.4f });
}

void HealerSKILLState::update(float elapsedTime, uint32_t currentFrame)
{
	StateMachine::update(elapsedTime, currentFrame);
}

void HealerSKILLState::exit()
{
	StateMachine::exit();
}

HealerObject::HealerObject(GLTFModelObject& mapObject, vkf::Effect& effect)
	: PlayerObject{ mapObject }, effect{ effect }
{
}

void HealerObject::initialize()
{
}

void HealerObject::update(float elapsedTime, uint32_t currentFrame)
{
	PlayerObject::update(elapsedTime, currentFrame);

	for (auto& mEffect : healerEffects) {
		mEffect.accumTime += elapsedTime;
	}

	std::list<std::vector<HealerObject::HealerEffect>::iterator> deleteEffects;
	for (auto itr = healerEffects.begin(); itr != healerEffects.end(); ++itr) {
		if (itr->accumTime >= 2.35f)
			deleteEffects.emplace_back(itr);
	}
	for (const auto& itr : deleteEffects) {
		healerEffects.erase(itr);
	}
}

void HealerObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	PlayerObject::draw(commandBuffer, pipelineLayout, currentFrame);
}

void HealerObject::release()
{
}

void HealerObject::drawEffect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	if (healerEffects.size() > 0) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &effect.texture.samplerDescriptorSet, 0, nullptr);

		for (const auto& mEffect : healerEffects) {
			if (mEffect.accumTime >= 0.f) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.f), mEffect.pos);
				matrix[3][3] = mEffect.accumTime;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);

				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			}
		}
	}
}

void HealerObject::changeATTACKState()
{
	currentState->exit();
	currentState = std::make_unique<HealerATTACKState>(*this);
	currentState->enter();
}

void HealerObject::changeSKILLState()
{
	currentState->exit();
	currentState = std::make_unique<HealerSKILLState>(*this);
	currentState->enter();
}
