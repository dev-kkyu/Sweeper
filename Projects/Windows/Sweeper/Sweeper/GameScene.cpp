#include "GameScene.h"
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "WarriorObject.h"
#include "ArcherObject.h"
#include "MageObject.h"
#include "HealerObject.h"
#include "MonsterObject.h"
#include "ArrowObject.h"

#include "NetworkManager.h"

GameScene::GameScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet, VkExtent2D& framebufferExtent)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, shadowSetLayout{ shadowSetLayout }, shadowSet{ shadowSet }
	, camera{ framebufferExtent }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createSamplerDescriptorPool(11);		// 배경 구름, 게임종료 2개, 이펙트7개, 힐러 파티클
	// 힐러 파티클 생성
	createParticle(50);

	uniformBufferObject.scene.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);
	uniformBufferObject.offscreen.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);

	// pool 개수 조절 필수
	// 배경 사각형 텍스처 생성
	cloudTexture.loadFromFile(fDevice, "models/Textures/cloud.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);

	gameendTexture[0].loadFromFile(fDevice, "models/Textures/gameover.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	gameendTexture[1].loadFromFile(fDevice, "models/Textures/gameclear.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);

	// 캐릭터 Effect 생성
	effect.warrior.texture.loadFromFile(fDevice, "models/Textures/smoke.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.archer.texture.loadFromFile(fDevice, "models/Textures/tornado.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.healer.texture.loadFromFile(fDevice, "models/Textures/healcircle.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.mage.attack.texture.loadFromFile(fDevice, "models/Textures/magic.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.mage.skill.texture.loadFromFile(fDevice, "models/Textures/magiccircle.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.arrow.texture.loadFromFile(fDevice, "models/Textures/arroweffect.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);
	effect.boss.texture.loadFromFile(fDevice, "models/Textures/bossmagic.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);

	// 힐러 파티클 이미지
	particleTexture.loadFromFile(fDevice, "models/Textures/particle.png", sceneSamplerDescriptorPool, descriptorSetLayout.sampler);

	// gltf 모델 로드
	mapModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/map.glb");
	// 화살 모델 로드
	arrowModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Arrow.glb");

	// gltf skin모델 로드
	// 몬스터 모델 로드
	monsterModel[static_cast<int>(MONSTER_TYPE::MUSHROOM)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Monster/Mushroom.glb");
	monsterModel[static_cast<int>(MONSTER_TYPE::BORNDOG)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Monster/BornDog.glb");
	monsterModel[static_cast<int>(MONSTER_TYPE::GOBLIN)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Monster/Goblin.glb");
	monsterModel[static_cast<int>(MONSTER_TYPE::BOOGIE)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Monster/Boogie.glb");
	bossModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/Monster/Boss_Golem.glb");
	// 플레이어 모델 로드
	playerModel[static_cast<int>(PLAYER_TYPE::WARRIOR)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Dragoon.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::ARCHER)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Archer.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::MAGE)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Mage.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::HEALER)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Priest.glb");

	// 맵 생성
	mapObject.setModel(mapModel);

	// 게임 리셋 데이터
	isEnd = false;
	isEndPacketReceived = false;
	isWin = false;
	gameEndAfterTime = 0.f;
}

GameScene::~GameScene()
{
	// 플레이어 객체들은 shared_ptr이므로, 따로 삭제 X
	for (auto& model : playerModel) {		// 플레이어 모델들
		model.destroy();
	}

	bossModel.destroy();

	pMonsterObjects.clear();				// 몬스터 객체들
	for (auto& model : monsterModel) {
		model.destroy();					// 몬스터 모델들
	}

	arrowModel.destroy();					// 화살 모델

	mapModel.destroy();						// 맵 모델

	// 플레이어와 맵 오브젝트는 알아서 삭제

	uniformBufferObject.scene.destroy();
	uniformBufferObject.offscreen.destroy();

	// 힐러 파티클 Destroy
	vkDestroyBuffer(fDevice.logicalDevice, particleVertexBuffer, nullptr);
	vkFreeMemory(fDevice.logicalDevice, particleVertexBufferMemory, nullptr);
	particleTexture.destroy();				// 힐러 파티클 텍스처

	effect.warrior.texture.destroy();		// 전사 이펙트 텍스처
	effect.archer.texture.destroy();		// 궁수 이펙트 텍스처
	effect.healer.texture.destroy();		// 힐러 이펙트 텍스처
	effect.mage.attack.texture.destroy();	// 마법사 이펙트 텍스처
	effect.mage.skill.texture.destroy();	// 마법사 이펙트 텍스처
	effect.arrow.texture.destroy();			// 화살 이펙트 텍스처
	effect.boss.texture.destroy();			// 보스 이펙트 텍스처

	gameendTexture[0].destroy();			// 게임End 텍스처
	gameendTexture[1].destroy();			// 게임End 텍스처

	cloudTexture.destroy();					// 구름 텍스처

	vkDestroyDescriptorPool(fDevice.logicalDevice, sceneSamplerDescriptorPool, nullptr);

	vkDestroyPipeline(fDevice.logicalDevice, pipeline.particlePipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.boss.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.arrow.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.mage.attack.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.mage.skill.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.healer.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.archer.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, effect.warrior.pipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.cloudPipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.bossHpBarPipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.hpBarPipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.boundingBoxPipeline, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.scene.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.scene.skinModel, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.offscreen.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.offscreen.skinModel, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ubo, nullptr);
}

void GameScene::start(PLAYER_TYPE player_type)
{
	// 오브젝트 초기화
	pArrowObjects.clear();
	pMonsterObjects.clear();
	for (auto& player : pPlayers)
		player = nullptr;

	// 보스 오브젝트 초기화 및 생성
	pBossObject = std::make_unique<BossObject>(effect.boss);
	pBossObject->initModel(bossModel, descriptorSetLayout.ssbo);
	pBossObject->setPosition({ 12.25f, 0.f, 115.f });		// 서버와 동기화 해야함
	pBossObject->setLook({ 0.f, 0.f, -1.f });
	pBossObject->setScale(glm::vec3{ 2.25f });

	// 내 플레이어 초기화 및 생성
	switch (this->player_type = player_type)
	{
	case PLAYER_TYPE::WARRIOR:
		pMyPlayer = std::make_shared<WarriorObject>(mapObject, effect.warrior);
		break;
	case PLAYER_TYPE::ARCHER:
		pMyPlayer = std::make_shared<ArcherObject>(mapObject, effect.archer);
		break;
	case PLAYER_TYPE::MAGE:
		pMyPlayer = std::make_shared<MageObject>(mapObject, effect.mage.attack, effect.mage.skill);
		break;
	case PLAYER_TYPE::HEALER:
		pMyPlayer = std::make_shared<HealerObject>(mapObject, effect.healer);
		break;
	default:
		throw std::runtime_error("ADD PLAYER ERROR : INVALID TYPE!\n");
		break;
	}
	pMyPlayer->initModel(playerModel[static_cast<int>(player_type)], descriptorSetLayout.ssbo);
	pMyPlayer->setScale(glm::vec3(1.3f));
	camera.setPlayer(pMyPlayer);

	// 게임 리셋 데이터
	isEnd = false;
	isEndPacketReceived = false;
	isWin = false;
	gameEndAfterTime = 0.f;
}

void GameScene::update(float elapsedTime, uint32_t currentFrame)
{
	if (not isEnd) {
		if (isEndPacketReceived) {
			gameEndAfterTime += elapsedTime;
			if (gameEndAfterTime > 5.f) {
				isEnd = true;
			}
		}
	}

	sceneElapsedTime += elapsedTime;

	glm::vec3 playerPos = pMyPlayer->getPosition();
	lightPos = playerPos + glm::vec3(-2.f, 5.2f, -2.f);		// light는 플레이어 위치에 따라서 바뀐다

	// 카메라 업데이트
	camera.update(elapsedTime);

	// UBO 업데이트
	vkf::UniformBufferObject ubo{};
	// Scene의 변환행렬 계산
	ubo.view = camera.getView();
	ubo.projection = camera.getProjection();
	ubo.lightPos = lightPos;

	// Scene을 드로우 할 때, 그림자 계산을 위한 빛 공간의 변환행렬을 알고 있어야 한다
	glm::mat4 lightView = glm::lookAt(lightPos, playerPos + glm::vec3(2.f, 0.f, 2.f), glm::vec3(0.f, 1.f, 0.f));		// 빛은 플레이어를 비춘다 (그림자 연산)
	//glm::mat4 lightProjection = glm::perspective(glm::radians(45.f), 1.f, 1.f, 100.f);		// 종횡비는 가로세로 같다. near 값은 1.f로 한다
	glm::mat4 lightProjection = glm::ortho(-50.f, 50.f, -50.f, 50.f, -2.3f, 50.f);				// 직교투영이 현재 게임에 적합... 종횡비는 같아야 한다
	ubo.lightSpace = lightProjection * lightView;

	uniformBufferObject.scene.updateUniformBuffer(ubo, currentFrame);

	// 오프스크린에 draw 할 때는, 빛의 시점에서 장면을 그린다
	ubo.view = lightView;
	ubo.projection = lightProjection;
	uniformBufferObject.offscreen.updateUniformBuffer(ubo, currentFrame);

	// 화살 오브젝트들 업데이트 (보정 포함)
	for (auto& arr : pArrowObjects) {
		arr.second->update(elapsedTime, currentFrame);
	}

	// 맵은 업데이트 X

	// 오브젝트 업데이트
	pBossObject->update(elapsedTime, currentFrame);

	for (auto& m : pMonsterObjects) {
		m.second->update(elapsedTime, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player) {
			player->update(elapsedTime, currentFrame);
		}
	}
}

void GameScene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen)
{
	// if문을 쓰지 않기 위한 코드..
	int idx = !!static_cast<int>(isOffscreen);

	// shadow map bind (offscreen draw시에는 사용하지 않는다)
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &shadowSet, 0, nullptr);

	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.uboOnOff[idx].descriptorSets[currentFrame], 0, nullptr);

	// Model Object들
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.sceneOnOff[idx].model);

	mapObject.draw(commandBuffer, pipelineLayout, currentFrame);

	for (auto& arr : pArrowObjects) {
		arr.second->draw(commandBuffer, pipelineLayout, currentFrame);
	}

	// skinModel Object들
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.sceneOnOff[idx].skinModel);

	pBossObject->draw(commandBuffer, pipelineLayout, currentFrame);

	for (auto& m : pMonsterObjects) {
		m.second->draw(commandBuffer, pipelineLayout, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->draw(commandBuffer, pipelineLayout, currentFrame);
	}
}

void GameScene::drawUI(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	{	// 체력 바 그려주기
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.hpBarPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.scene.descriptorSets[currentFrame], 0, nullptr);

		for (auto& m : pMonsterObjects) {
			m.second->drawUI(commandBuffer, pipelineLayout);
		}
		for (auto& player : pPlayers) {
			if (player)
				player->drawUI(commandBuffer, pipelineLayout);
		}

		if (pMyPlayer) {
			BoundingBox bossArea;
			bossArea.setBound(1.f, 0.1f, 125.f, 107.f, 3.f, 21.f);
			if (bossArea.isCollide(pMyPlayer->getBoundingBox())) {		// 보스 영역 안에 있을때만 그려준다
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.bossHpBarPipeline);
				pBossObject->drawUI(commandBuffer, pipelineLayout);
			}
		}
	}

	{	// 배경 사각형 그리기
		glm::mat4 matrix{ 1.f };
		if (pMyPlayer) {
			matrix[3] = glm::vec4(pMyPlayer->getPosition(), 1.f);
		}
		matrix[3][3] = sceneElapsedTime;
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.cloudPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &cloudTexture.samplerDescriptorSet, 0, nullptr);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}

	// 게임 승리/패배 그려주기
	{
		if (isEndPacketReceived and gameEndAfterTime > 1.5f) {
			glm::mat4 matrix{ 1.f };
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.gameendPipeline);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &gameendTexture[static_cast<int>(isWin)].samplerDescriptorSet, 0, nullptr);
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
			vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		}
	}
}

void GameScene::drawEffect(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.scene.descriptorSets[currentFrame], 0, nullptr);

	for (auto& player : pPlayers) {
		if (player)
			player->drawEffect(commandBuffer, pipelineLayout);
	}

	// 화살의 이펙트도 같이 그려준다
	for (const auto& arr : pArrowObjects) {
		arr.second->drawEffect(commandBuffer, pipelineLayout);
	}

	pBossObject->drawEffect(commandBuffer, pipelineLayout);

	// 힐러 스킬의 대상 플레이어 주위에 그려주는 파티클
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.particlePipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &particleTexture.samplerDescriptorSet, 0, nullptr);
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &particleVertexBuffer, offsets);

		for (const auto& player : pPlayers) {
			if (player) {									// 모든 플레이어 중에 힐러를 찾는다
				if (player->getPlayerType() == PLAYER_TYPE::HEALER) {
					auto healer = dynamic_cast<HealerObject*>(player.get());
					const auto& healerEffects = healer->getHealerEffects();

					// 힐러의 이펙트를 찾는다
					for (const auto& hEffect : healerEffects) {
						if (hEffect.accumTime >= 0.f) {
							for (int i = 0; i < 4; ++i) {		// 해당 이펙트 위에 있는 플레이어를 다시 찾는다
								if (pPlayers[i]) {
									auto playerPos = pPlayers[i]->getPosition();
									float dist = glm::length(playerPos - hEffect.pos);
									if (dist <= pPlayers[i]->getCollisionRadius() + 2.f) {
										// 이펙트로부터 반지름 2.f 안에 있다면 그려준다

										glm::mat4 matrix = glm::translate(glm::mat4(1.f), pPlayers[i]->getPosition());
										matrix[3][3] = sceneElapsedTime;	// 여기에 시간 포함에서 넘겨주기
										vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkf::PushConstantData), &matrix);
										// set = 1에 샘플러 바인드
										vkCmdDraw(commandBuffer, particleVertexCount, 1, 0, 0);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void GameScene::drawBoundingBox(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.boundingBoxPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.scene.descriptorSets[currentFrame], 0, nullptr);
	mapObject.drawBoundingBox(commandBuffer, pipelineLayout);
	for (const auto& player : pPlayers) {
		if (player)
			player->drawBoundingBox(commandBuffer, pipelineLayout);
	}
	for (const auto& arr : pArrowObjects) {
		arr.second->drawBoundingBox(commandBuffer, pipelineLayout);
	}
}

void GameScene::processKeyboard(int key, int action, int mods)
{
	// 키 입력을 서버로 전송한다.

	CS_KEY_EVENT_PACKET p;
	p.size = sizeof(p);
	p.type = CS_KEY_EVENT;
	switch (action)
	{
	case GLFW_PRESS:
		p.is_pressed = true;
		switch (key)
		{
		case GLFW_KEY_W:
			p.key = KEY_UP;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_A:
			p.key = KEY_LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_S:
			p.key = KEY_DOWN;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_D:
			p.key = KEY_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_SPACE:
			p.key = KEY_SPACE;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_LEFT_CONTROL:
			p.key = KEY_CTRL;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_P:
			if (pMyPlayer) {	// P 키 누를 시 플레이어의 위치를 콘솔에 출력한다
				auto pos = pMyPlayer->getPosition();
				std::cout << "Position - (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
			}
			break;
		case GLFW_KEY_LEFT:
			if (pMyPlayer) {
				if (pMyPlayer->getPlayerState() == PLAYER_STATE::DIE) {
					for (int i = 0; i < 4; ++i) {
						--observer_id;
						if (observer_id < 0)
							observer_id = 3;
						if (pPlayers[observer_id]) {
							camera.setPlayer(pPlayers[observer_id]);
							break;
						}
					}
				}
			}
			break;
		case GLFW_KEY_RIGHT:
			if (pMyPlayer) {
				if (pMyPlayer->getPlayerState() == PLAYER_STATE::DIE) {
					for (int i = 0; i < 4; ++i) {
						++observer_id;
						if (observer_id > 3)
							observer_id = 0;
						if (pPlayers[observer_id]) {
							camera.setPlayer(pPlayers[observer_id]);
							break;
						}
					}
				}
			}
			break;
		}	// switch (key)
		break;
	case GLFW_RELEASE:
		p.is_pressed = false;
		switch (key)
		{
		case GLFW_KEY_W:
			p.key = KEY_UP;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_A:
			p.key = KEY_LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_S:
			p.key = KEY_DOWN;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_D:
			p.key = KEY_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_SPACE:
			p.key = KEY_SPACE;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_LEFT_CONTROL:
			p.key = KEY_CTRL;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		break;
	}
}

void GameScene::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	switch (action)
	{
	case GLFW_PRESS:
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = true;
			p.key = MOUSE_LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = true;
			p.key = MOUSE_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
			middleButtonPressed = true;
			camera.setStartMousePos(xpos, ypos);			// 위아래 회전용 (위아래 회전은 플레이어 방향에 영향을 주지 않는다)
			break;
		case GLFW_MOUSE_BUTTON_4:
			break;
		case GLFW_MOUSE_BUTTON_5:
			break;
		}
		break;
	case GLFW_RELEASE:
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = false;
			p.key = MOUSE_LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = false;
			p.key = MOUSE_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
			middleButtonPressed = false;
			break;
		case GLFW_MOUSE_BUTTON_4:
			break;
		case GLFW_MOUSE_BUTTON_5:
			break;
		}
		break;
	}
}

void GameScene::processMouseScroll(double xoffset, double yoffset)
{
	float distance = camera.getDistance();
	if (yoffset > 0.f) {
		if (distance > 4.f)
			camera.setDistance(distance - 1.f);
	}
	else if (yoffset < 0.f) {
		camera.setDistance(distance + 1.f);
	}
}

void GameScene::processMouseCursor(float xpos, float ypos)
{
	if (middleButtonPressed) {
		camera.processMouseCursor(xpos, ypos);
	}
}

void GameScene::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case SC_LOGIN_INFO: {
		auto p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		std::cout << "로그인 패킷 수신, ROOM:ID->[" << int(p->room_id) << ":" << int(p->player_id) << "]\n";
		// 룸 ID는 아직 사용하지 않음
		my_id = p->player_id;
		observer_id = my_id;
		pMyPlayer->setPosition(glm::vec3(p->pos_x, 0.f, p->pos_z));
		pMyPlayer->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		pPlayers[my_id] = pMyPlayer;
		break;
	}
	case SC_LOGOUT: {
		auto p = reinterpret_cast<SC_LOGOUT_PACKET*>(packet);
		std::cout << "로그아웃 패킷 수신 ID:[" << int(p->player_id) << "]\n";
		vkDeviceWaitIdle(fDevice.logicalDevice);	// Vulkan 호출하는 오브젝트 삭제 전에는 무조건 해줘야 한다.
		pPlayers[p->player_id] = nullptr;
		break;
	}
	case SC_ADD_PLAYER: {
		auto p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);
		std::cout << "플레이어 추가 패킷 수신 ID:[" << int(p->player_id) << "]\n";
		switch (p->player_type)
		{
		case PLAYER_TYPE::WARRIOR:
			pPlayers[p->player_id] = std::make_shared<WarriorObject>(mapObject, effect.warrior);
			break;
		case PLAYER_TYPE::ARCHER:
			pPlayers[p->player_id] = std::make_shared<ArcherObject>(mapObject, effect.archer);
			break;
		case PLAYER_TYPE::MAGE:
			pPlayers[p->player_id] = std::make_shared<MageObject>(mapObject, effect.mage.attack, effect.mage.skill);
			break;
		case PLAYER_TYPE::HEALER:
			pPlayers[p->player_id] = std::make_shared<HealerObject>(mapObject, effect.healer);
			break;
		default:
			throw std::runtime_error("ADD PLAYER ERROR : INVALID TYPE!\n");
			break;
		}
		pPlayers[p->player_id]->setHP(p->hp);
		pPlayers[p->player_id]->initModel(playerModel[static_cast<int>(p->player_type)], descriptorSetLayout.ssbo);
		pPlayers[p->player_id]->setScale(glm::vec3(1.3f));
		pPlayers[p->player_id]->setPosition(glm::vec3(p->pos_x, 0.f, p->pos_z));
		pPlayers[p->player_id]->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		break;
	}
	case SC_MOVE_PLAYER: {
		auto p = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(packet);
		pPlayers[p->player_id]->setPosition(glm::vec3(p->pos_x, p->pos_y, p->pos_z));
		pPlayers[p->player_id]->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		break;
	}
	case SC_PLAYER_LOOK: {
		auto p = reinterpret_cast<SC_PLAYER_LOOK_PACKET*>(packet);
		pPlayers[p->player_id]->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		break;
	}
	case SC_PLAYER_STATE: {
		auto p = reinterpret_cast<SC_PLAYER_STATE_PACKET*>(packet);
		std::string state;
		switch (p->state) {
		case PLAYER_STATE::IDLE:
			pPlayers[p->player_id]->changeIDLEState();
			state = "IDLE";
			break;
		case PLAYER_STATE::RUN:
			pPlayers[p->player_id]->changeRUNState();
			state = "RUN";
			break;
		case PLAYER_STATE::DASH:
			pPlayers[p->player_id]->changeDASHState();
			state = "DASH";
			break;
		case PLAYER_STATE::HIT:
			pPlayers[p->player_id]->changeHITState();
			state = "HIT";
			break;
		case PLAYER_STATE::DIE:
			pPlayers[p->player_id]->changeDIEState();
			state = "DIE";
			break;
		case PLAYER_STATE::ATTACK:
			pPlayers[p->player_id]->changeATTACKState();
			state = "ATTACK";
			break;
		case PLAYER_STATE::SKILL:
			pPlayers[p->player_id]->changeSKILLState();
			state = "SKILL";
			break;
		default:
			std::cout << int(p->player_id) << ": STATE 에러" << std::endl;
			break;
		}
		if (not state.empty())
			std::cout << "PLAYER[" << int(p->player_id) << "]의 상태가 " << state << "로 변경" << std::endl;
		break;
	}
	case SC_PLAYER_HP: {
		auto p = reinterpret_cast<SC_PLAYER_HP_PACKET*>(packet);
		if (pPlayers[p->player_id])
			pPlayers[p->player_id]->setHP(p->hp);
		break;
	}
	case SC_ADD_ARROW: {
		auto p = reinterpret_cast<SC_ADD_ARROW_PACKET*>(packet);
		if (pArrowObjects.find(p->arrow_id) == pArrowObjects.end()) {
			pArrowObjects[p->arrow_id] = std::make_shared<ArrowObject>(effect.arrow);
			pArrowObjects[p->arrow_id]->setModel(arrowModel);
			pArrowObjects[p->arrow_id]->setPosition(glm::vec3(p->pos_x, 0.75f, p->pos_z));
			pArrowObjects[p->arrow_id]->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
			pArrowObjects[p->arrow_id]->setScale(glm::vec3(3.f));
			std::cout << "화살 [" << int(p->arrow_id) << "] 추가" << std::endl;
		}
		else {
			std::cerr << "ERROR : SC_ADD_ARROW - 이미 존재하는 오브젝트!" << std::endl;
		}
		break;
	}
	case SC_MOVE_ARROW: {
		auto p = reinterpret_cast<SC_MOVE_ARROW_PACKET*>(packet);
		if (pArrowObjects.find(p->arrow_id) != pArrowObjects.end()) {
			pArrowObjects[p->arrow_id]->setPosition(glm::vec3(p->pos_x, 0.75f, p->pos_z));
		}
		else {
			std::cerr << "ERROR : SC_MOVE_ARROW - 존재하지 않는 오브젝트!" << std::endl;
		}
		break;
	}
	case SC_REMOVE_ARROW: {
		auto p = reinterpret_cast<SC_REMOVE_ARROW_PACKET*>(packet);
		vkDeviceWaitIdle(fDevice.logicalDevice);	// Vulkan 호출하는 오브젝트 삭제 전에는 무조건 해줘야 한다.
		pArrowObjects.erase(p->arrow_id);
		std::cout << "화살 [" << int(p->arrow_id) << "] 제거" << std::endl;
		break;
	}
	case SC_CLIENT_KEY_EVENT: {
		auto p = reinterpret_cast<SC_CLIENT_KEY_EVENT_PACKET*>(packet);
		pPlayers[p->player_id]->processKeyInput(p->key, p->is_pressed);
		break;
	}
	case SC_ADD_MONSTER: {
		auto p = reinterpret_cast<SC_ADD_MONSTER_PACKET*>(packet);
		pMonsterObjects.try_emplace(p->monster_id, std::make_shared<MonsterObject>());
		switch (p->monster_type)
		{
		case MONSTER_TYPE::MUSHROOM:
			pMonsterObjects[p->monster_id]->setMaxHP(MAX_HP_MONSTER_MUSHROOM);
			break;
		case MONSTER_TYPE::BORNDOG:
			pMonsterObjects[p->monster_id]->setMaxHP(MAX_HP_MONSTER_BORNDOG);
			break;
		case MONSTER_TYPE::GOBLIN:
			pMonsterObjects[p->monster_id]->setScale(glm::vec3(1.3f));
			pMonsterObjects[p->monster_id]->setHeadHeight(1.5f);
			pMonsterObjects[p->monster_id]->setMaxHP(MAX_HP_MONSTER_GOBLIN);
			break;
		case MONSTER_TYPE::BOOGIE:
			pMonsterObjects[p->monster_id]->setHeadHeight(1.5f);
			pMonsterObjects[p->monster_id]->setMaxHP(MAX_HP_MONSTER_BOOGIE);
			break;
		default:
			std::cout << "ERROR: INVALID MONSTER TYPE!" << std::endl;
			break;
		}
		pMonsterObjects[p->monster_id]->setHP(p->hp);
		pMonsterObjects[p->monster_id]->initModel(monsterModel[static_cast<int>(p->monster_type)], descriptorSetLayout.ssbo);
		pMonsterObjects[p->monster_id]->setPosition({ p->pos_x, 0.f, p->pos_z });
		pMonsterObjects[p->monster_id]->setLook({ p->dir_x, 0.f, p->dir_z });
		std::cout << "몬스터 [" << int(p->monster_id) << "], 타입 [" << static_cast<int>(p->monster_type) << "] 추가" << std::endl;
		break;
	}
	case SC_MOVE_MONSTER: {
		auto p = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(packet);
		pMonsterObjects[p->monster_id]->setPosition({ p->pos_x, 0.f, p->pos_z });
		pMonsterObjects[p->monster_id]->setLook({ p->dir_x, 0.f, p->dir_z });
		break;
	}
	case SC_MONSTER_LOOK: {
		auto p = reinterpret_cast<SC_MONSTER_LOOK_PACKET*>(packet);
		pMonsterObjects[p->monster_id]->setLook({ p->dir_x, 0.f, p->dir_z });
		break;
	}
	case SC_REMOVE_MONSTER: {
		auto p = reinterpret_cast<SC_REMOVE_MONSTER_PACKET*>(packet);
		vkDeviceWaitIdle(fDevice.logicalDevice);	// Vulkan 호출하는 오브젝트 삭제 전에는 무조건 해줘야 한다.
		pMonsterObjects.erase(p->monster_id);
		std::cout << "몬스터 [" << int(p->monster_id) << "] 제거" << std::endl;
		break;
	}
	case SC_MONSTER_STATE: {
		auto p = reinterpret_cast<SC_MONSTER_STATE_PACKET*>(packet);
		switch (p->state)
		{
		case MONSTER_STATE::IDLE:
		case MONSTER_STATE::MOVE:
		case MONSTER_STATE::HIT:
		case MONSTER_STATE::DIE:
		case MONSTER_STATE::ATTACK:
			pMonsterObjects[p->monster_id]->setAnimationClip(static_cast<int>(p->state));
			break;
		default:
			std::cout << int(p->monster_id) << ": STATE 에러" << std::endl;
			break;
		}
		std::string state_str[5]{ "IDLE", "MOVE", "HIT", "DIE", "ATTACK" };
		std::cout << "몬스터 " << int(p->monster_id) << "의 상태가 "
			<< state_str[static_cast<int>(p->state)] << "로 변경" << std::endl;
		break;
	}
	case SC_MONSTER_HP: {
		auto p = reinterpret_cast<SC_MONSTER_HP_PACKET*>(packet);
		if (0 != pMonsterObjects.count(p->monster_id))
			pMonsterObjects[p->monster_id]->setHP(p->hp);
		break;
	}
	case SC_MOVE_BOSS: {
		auto p = reinterpret_cast<SC_MOVE_BOSS_PACKET*>(packet);
		pBossObject->setPosition(glm::vec3{ p->pos_x, 0.f, p->pos_z });
		pBossObject->setLook(glm::vec3{ p->dir_x, 0.f, p->dir_z });
		break;
	}
	case SC_BOSS_STATE: {
		auto p = reinterpret_cast<SC_BOSS_STATE_PACKET*>(packet);
		pBossObject->setBossState(p->state);
		break;
	}
	case SC_BOSS_HP: {
		auto p = reinterpret_cast<SC_BOSS_HP_PACKET*>(packet);
		pBossObject->setHP(p->hp);
		break;
	}
	case SC_GAME_END: {
		auto p = reinterpret_cast<SC_GAME_END_PACKET*>(packet);
		isEndPacketReceived = true;
		isWin = p->is_win;
		if (p->is_win) {
			std::cout << "게임 승리 패킷 수신!" << std::endl;
		}
		else {
			std::cout << "게임 패배 패킷 수신!" << std::endl;
		}
		break;
	}
	default:
		std::cerr << "ERROR : INVALID PACKET ON SCENE!" << std::endl;
		break;
	}
}

PLAYER_TYPE GameScene::getPlayerType() const
{
	return player_type;
}

std::array<VulkanGLTFSkinModel, 4>& GameScene::getPlayerModel()
{
	return playerModel;
}

VkDescriptorSetLayout GameScene::getUBODescriptorSetLayout() const
{
	return descriptorSetLayout.ubo;
}

VkDescriptorSetLayout GameScene::getSamplerDescriptorSetLayout() const
{
	return descriptorSetLayout.sampler;
}

VkDescriptorSetLayout GameScene::getSSBODescriptorSetLayout() const
{
	return descriptorSetLayout.ssbo;
}

VkPipelineLayout GameScene::getPipelineLayout() const
{
	return pipelineLayout;
}

VkPipeline GameScene::getModelPipeline() const
{
	return pipeline.scene.model;
}

VkPipeline GameScene::getSkinModelPipeline() const
{
	return pipeline.scene.skinModel;
}

VkPipeline GameScene::getOffscreenModelPipeline() const
{
	return pipeline.offscreen.model;
}

VkPipeline GameScene::getOffscreenSkinModelPipeline() const
{
	return pipeline.offscreen.skinModel;
}

bool GameScene::getIsEnd() const
{
	return isEnd;
}

void GameScene::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding ssboLayoutBinding{};
	ssboLayoutBinding.binding = 0;
	ssboLayoutBinding.descriptorCount = 1;
	ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ssboLayoutBinding.pImmutableSamplers = nullptr;
	ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.ubo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = &samplerLayoutBinding;
	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = &ssboLayoutBinding;
	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.ssbo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void GameScene::createGraphicsPipeline()
{
	vkf::Shader modelShader{ fDevice, "shaders/model.vert.spv", "shaders/fragment.frag.spv" };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = vkf::Vertex::getBindingDescription();
	auto attributeDescriptions = vkf::Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = msaaSamples;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE; // 블렌딩 활성화

	// 알파 블렌딩 설정
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;				// 소스 알파
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;		// 대상 알파
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;								// 블렌딩 연산

	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;						// 소스 알파 값 유지
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;					// 대상 알파에 영향을 주지 않음
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;								// 알파 블렌딩 연산

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// 여러 개의 디스크립터 셋을 사용할 때, set의 index를 pSetLayouts의 index와 맞춰줘야 한다.
	std::vector<VkDescriptorSetLayout> setLayout{ 4 };
	setLayout[0] = shadowSetLayout;
	setLayout[1] = descriptorSetLayout.sampler;
	setLayout[2] = descriptorSetLayout.ubo;
	setLayout[3] = descriptorSetLayout.ssbo;										// skinModel에서만 사용

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(vkf::PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayout.size());	// model에서는 (0, 1, 2) 사용, offscreen은 0번 X
	pipelineLayoutInfo.pSetLayouts = setLayout.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(modelShader.shaderStages.size());
	pipelineInfo.pStages = modelShader.shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass.scene;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.scene.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// skinModel용 pipeline 생성
	vkf::Shader skinModelShader{ fDevice, "shaders/skinnedmodel.vert.spv", "shaders/fragment.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(skinModelShader.shaderStages.size());
	pipelineInfo.pStages = skinModelShader.shaderStages.data();

	auto skinBindingDescription = vkf::SkinVertex::getBindingDescription();
	auto skinAttributeDescriptions = vkf::SkinVertex::getAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &skinBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(skinAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = skinAttributeDescriptions.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.scene.skinModel) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// offscreen 파이프라인 생성
	{
		// 그림자 생성시에는 멀티샘플링 Off
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// No blend attachment states (no color attachments used)
		colorBlending.attachmentCount = 0;
		// Disable culling, so all faces contribute to shadows
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;			// 그림자 샘플에서는 기본 OP도 이걸로 되어있다
		// Enable depth bias
		rasterizer.depthBiasEnable = VK_TRUE;
		// Add depth bias to dynamic state, so we can change it at runtime
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		pipelineInfo.renderPass = renderPass.offscreen;
	}
	// skinModel, 위에서 pStages는 skinModel Shader로 연결되어 있다
	pipelineInfo.stageCount = 1;		// vertex shader만 사용

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.offscreen.skinModel) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// Model
	pipelineInfo.pStages = modelShader.shaderStages.data();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.offscreen.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// BoundingBox
	vkf::Shader boundingBoxShader{ fDevice, "shaders/boundingbox.vert.spv", "shaders/boundingbox.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(boundingBoxShader.shaderStages.size());
	pipelineInfo.pStages = boundingBoxShader.shaderStages.data();

	// input이 없는 쉐이더
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// offscreen에서 바뀐부분 원복
	multisampling.rasterizationSamples = msaaSamples;
	colorBlending.attachmentCount = 1;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	rasterizer.depthBiasEnable = VK_FALSE;
	dynamicState.dynamicStateCount = 2;		// offscreen에서 추가한거 다시 제거
	pipelineInfo.renderPass = renderPass.scene;

	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.boundingBoxPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// 다시 삼각형 리스트로 변경
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// 체력 바 파이프라인 생성
	vkf::Shader hpBarShader{ fDevice, "shaders/hpbar.vert.spv", "shaders/hpbar.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(hpBarShader.shaderStages.size());
	pipelineInfo.pStages = hpBarShader.shaderStages.data();

	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_FALSE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.hpBarPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader bossHpBarShader{ fDevice, "shaders/bosshpbar.vert.spv", "shaders/hpbar.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(bossHpBarShader.shaderStages.size());
	pipelineInfo.pStages = bossHpBarShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.bossHpBarPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// 배경 구름 파이프라인 생성
	vkf::Shader cloudShader{ fDevice, "shaders/cloud.vert.spv", "shaders/cloud.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(cloudShader.shaderStages.size());
	pipelineInfo.pStages = cloudShader.shaderStages.data();

	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.cloudPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// 게임 승리/패배 파이프라인 생성
	vkf::Shader gameendShader{ fDevice, "shaders/startscene.vert.spv", "shaders/startscene.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(gameendShader.shaderStages.size());
	pipelineInfo.pStages = gameendShader.shaderStages.data();

	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.gameendPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader warriorShader{ fDevice, "shaders/warriorskill.vert.spv", "shaders/warriorskill.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(warriorShader.shaderStages.size());
	pipelineInfo.pStages = warriorShader.shaderStages.data();

	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_FALSE;	// 이펙트는 Depth Write Off, 이펙트끼리 중첩하도록
	rasterizer.cullMode = VK_CULL_MODE_NONE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.warrior.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader archerShader{ fDevice, "shaders/archerskill.vert.spv", "shaders/archerskill.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(archerShader.shaderStages.size());
	pipelineInfo.pStages = archerShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.archer.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader healerShader{ fDevice, "shaders/healerskill.vert.spv", "shaders/healerskill.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(healerShader.shaderStages.size());
	pipelineInfo.pStages = healerShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.healer.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader mageAttackShader{ fDevice, "shaders/mageattack.vert.spv", "shaders/mageattack.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(mageAttackShader.shaderStages.size());
	pipelineInfo.pStages = mageAttackShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.mage.attack.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader mageSkillShader{ fDevice, "shaders/mageskill.vert.spv", "shaders/mageskill.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(mageSkillShader.shaderStages.size());
	pipelineInfo.pStages = mageSkillShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.mage.skill.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader arrowShader{ fDevice, "shaders/arroweffect.vert.spv", "shaders/arroweffect.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(arrowShader.shaderStages.size());
	pipelineInfo.pStages = arrowShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.arrow.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader bossShader{ fDevice, "shaders/bossskill.vert.spv", "shaders/bossskill.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(bossShader.shaderStages.size());
	pipelineInfo.pStages = bossShader.shaderStages.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &effect.boss.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkf::Shader particleShader{ fDevice, "shaders/particle.vert.spv", "shaders/particle.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(particleShader.shaderStages.size());
	pipelineInfo.pStages = particleShader.shaderStages.data();

	auto particleBindingDescription = HealerParticleData::getBindingDescription();
	auto particleAttributeDescriptions = HealerParticleData::getAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &particleBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(particleAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = particleAttributeDescriptions.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.particlePipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void GameScene::createSamplerDescriptorPool(uint32_t setCount)
{
	// 씬의 텍스처(구름 등) 샘플러에 사용할 descriptor pool 할당
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = setCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = setCount;

	if (vkCreateDescriptorPool(fDevice.logicalDevice, &poolInfo, nullptr, &sceneSamplerDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void GameScene::createParticle(int particleCount)
{
	std::vector<HealerParticleData> vertices;

	int verticesCount = particleCount * 6;
	vertices.reserve(verticesCount);

	for (int i = 0; i < particleCount; ++i) {
		// 원통 만들어주기
		float randRadianAngle = rand() / float(RAND_MAX) * 2.f * glm::pi<float>();	// 0 ~ 2pi
		float radius = 0.75f;
		glm::vec3 pos{
			radius * rand() / float(RAND_MAX) * glm::cos(randRadianAngle),
			rand() / float(RAND_MAX),
			radius * rand() / float(RAND_MAX) * glm::sin(randRadianAngle) };
		float emitTime = rand() / float(RAND_MAX) * 0.5f;			// 0 ~ 0.5초
		float lifeTime = rand() / float(RAND_MAX) * 0.5f + 0.5f;	// 0.5 ~ 1초

		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
		vertices.push_back(HealerParticleData{ pos, emitTime, lifeTime });
	}
	particleVertexCount = static_cast<uint32_t>(vertices.size());		// 버텍스 개수 지정

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(fDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(fDevice.logicalDevice, stagingBufferMemory);

	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particleVertexBuffer, particleVertexBufferMemory);

	vkf::copyBuffer(fDevice, stagingBuffer, particleVertexBuffer, bufferSize);

	vkDestroyBuffer(fDevice.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(fDevice.logicalDevice, stagingBufferMemory, nullptr);
}

HealerParticleData::HealerParticleData(glm::vec3 pos, float emitTime, float lifeTime)
	: pos{ pos }, emitTime{ emitTime }, lifeTime{ lifeTime }
{
}

VkVertexInputBindingDescription HealerParticleData::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(HealerParticleData);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> HealerParticleData::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(HealerParticleData, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(HealerParticleData, emitTime);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(HealerParticleData, lifeTime);

	return attributeDescriptions;
}
