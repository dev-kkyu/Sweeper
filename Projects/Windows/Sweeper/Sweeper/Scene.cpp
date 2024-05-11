#include "Scene.h"
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "NetworkManager.h"

#define CLIP_IDLE			19
#define CLIP_RUN			24
#define CLIP_ATTACK_KNIFE	10

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout, VkDescriptorSet& shadowSet)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, shadowSetLayout{ shadowSetLayout }, shadowSet{ shadowSet }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	uniformBufferObject.scene.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);
	uniformBufferObject.offscreen.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);

	// gltf �� �ε�
	mapModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/map.glb");

	// gltf skin�� �ε�
	mushroomModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/blue_mushroom.glb");
	bossModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/boss_golem.glb");
	// �÷��̾� �� �ε� (skin model)
	playerModel[static_cast<int>(PLAYER_TYPE::WARRIOR)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Dragoon.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::ARCHER)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Archer.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::MAGE)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Mage.glb");
	playerModel[static_cast<int>(PLAYER_TYPE::HEALER)].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Priest.glb");

	// �� ����
	mapObject.setModel(mapModel);

	// ���� �� ����
	bossObject.initModel(bossModel, descriptorSetLayout.ssbo);
	bossObject.setAnimationClip(2);
	bossObject.setPosition({ 12.25f, -0.8f, 115.f });
	bossObject.setLook({ 0.f, 0.f, -1.f });
	bossObject.setScale(glm::vec3{ 2.25f });
	bossObject.setAnimateSpeed(0.7f);

	// �÷��̾� ���� �� ����
	{
		int sel_type{};
		std::cout << "�÷��̾� ���� (0~3): ";
		std::cin >> sel_type;
		if (std::cin.fail() or sel_type < 0 or sel_type > 3) {
			std::cout << "�߸��� �Է�" << std::endl;
			exit(-1);
		}
		player_type = static_cast<PLAYER_TYPE>(sel_type);

		pMyPlayer = std::make_shared<PlayerObject>();		// Todo : ���� Ÿ�Կ� ���� �ٸ��� ����
		pMyPlayer->initModel(playerModel[static_cast<int>(player_type)], descriptorSetLayout.ssbo);
		pMyPlayer->setScale(glm::vec3(1.3f));
		pMyPlayer->setAnimationClip(CLIP_IDLE);	// Idle
		camera.setPlayer(pMyPlayer);
	}

}

Scene::~Scene()
{
	// �÷��̾� ��ü���� shared_ptr�̹Ƿ�, ���� ���� X
	for (auto& model : playerModel) {		// �÷��̾� �𵨵�
		model.destroy();
	}

	bossModel.destroy();

	pMonsterObjects.clear();				// ���� ��ü��
	mushroomModel.destroy();				// ����-���� ��

	mapModel.destroy();						// �� ��

	// �÷��̾�� �� ������Ʈ�� �˾Ƽ� ����

	uniformBufferObject.scene.destroy();
	uniformBufferObject.offscreen.destroy();

	vkDestroyPipeline(fDevice.logicalDevice, pipeline.scene.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.scene.skinModel, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.offscreen.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.offscreen.skinModel, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ubo, nullptr);
}

void Scene::update(float elapsedTime, uint32_t currentFrame)
{
	glm::vec3 playerPos = pMyPlayer->getPosition();
	lightPos = playerPos + glm::vec3(-2.f, 5.2f, -2.f);		// light�� �÷��̾� ��ġ�� ���� �ٲ��

	// ī�޶� ������Ʈ
	camera.update(elapsedTime);

	// UBO ������Ʈ
	vkf::UniformBufferObject ubo{};
	// Scene�� ��ȯ��� ���
	ubo.view = camera.getView();
	ubo.projection = camera.getProjection();
	ubo.lightPos = lightPos;

	// Scene�� ��ο� �� ��, �׸��� ����� ���� �� ������ ��ȯ����� �˰� �־�� �Ѵ�
	glm::mat4 lightView = glm::lookAt(lightPos, playerPos + glm::vec3(2.f, 0.f, 2.f), glm::vec3(0.f, 1.f, 0.f));		// ���� �÷��̾ ����� (�׸��� ����)
	//glm::mat4 lightProjection = glm::perspective(glm::radians(45.f), 1.f, 1.f, 100.f);		// ��Ⱦ��� ���μ��� ����. near ���� 1.f�� �Ѵ�
	glm::mat4 lightProjection = glm::ortho(-50.f, 50.f, -50.f, 50.f, -2.3f, 50.f);				// ���������� ���� ���ӿ� ����... ��Ⱦ��� ���ƾ� �Ѵ�
	ubo.lightSpace = lightProjection * lightView;

	uniformBufferObject.scene.updateUniformBuffer(ubo, currentFrame);

	// ������ũ���� draw �� ����, ���� �������� ����� �׸���
	ubo.view = lightView;
	ubo.projection = lightProjection;
	uniformBufferObject.offscreen.updateUniformBuffer(ubo, currentFrame);

	// ���� ������Ʈ X

	// ������Ʈ ������Ʈ
	bossObject.update(elapsedTime, currentFrame);

	for (auto& m : pMonsterObjects) {
		m.second->update(elapsedTime, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->update(elapsedTime, currentFrame);
	}
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen)
{
	// if���� ���� �ʱ� ���� �ڵ�..
	Pipeline::Type* lineType[2]{ &pipeline.scene, &pipeline.offscreen };
	vkf::BufferObject* uboType[2]{ &uniformBufferObject.scene, &uniformBufferObject.offscreen };
	int idx = !!static_cast<int>(isOffscreen);
	Pipeline::Type& line = *lineType[idx];
	vkf::BufferObject& ubo = *uboType[idx];

	// shadow map bind (offscreen draw�ÿ��� ������� �ʴ´�)
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &shadowSet, 0, nullptr);

	// UBO ���ε�, firstSet�� set�� �����ε���
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &ubo.descriptorSets[currentFrame], 0, nullptr);

	// Model Object��
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, line.model);

	mapObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// skinModel Object��
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, line.skinModel);

	bossObject.draw(commandBuffer, pipelineLayout, currentFrame);

	for (auto& m : pMonsterObjects) {
		m.second->draw(commandBuffer, pipelineLayout, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->draw(commandBuffer, pipelineLayout, currentFrame);
	}
}

void Scene::processKeyboard(int key, int action, int mods)
{
	// Ű �Է��� ������ �����Ѵ�.

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
			p.key = MY_KEY_EVENT::UP;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_A:
			p.key = MY_KEY_EVENT::LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_S:
			p.key = MY_KEY_EVENT::DOWN;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_D:
			p.key = MY_KEY_EVENT::RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_SPACE:
			p.key = MY_KEY_EVENT::SPACE;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		break;
	case GLFW_RELEASE:
		p.is_pressed = false;
		switch (key)
		{
		case GLFW_KEY_W:
			p.key = MY_KEY_EVENT::UP;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_A:
			p.key = MY_KEY_EVENT::LEFT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_S:
			p.key = MY_KEY_EVENT::DOWN;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_D:
			p.key = MY_KEY_EVENT::RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		case GLFW_KEY_SPACE:
			p.key = MY_KEY_EVENT::SPACE;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		break;
	}
}

void Scene::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	switch (action)
	{
	case GLFW_PRESS:
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			leftButtonPressed = true;
			camera.setStartMousePos(xpos, ypos);			// ���Ʒ� ȸ���� (���Ʒ� ȸ���� �÷��̾� ���⿡ ������ ���� �ʴ´�)
			break;
		case GLFW_MOUSE_BUTTON_RIGHT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = true;
			p.key = MY_KEY_EVENT::MOUSE_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
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
		case GLFW_MOUSE_BUTTON_LEFT:
			leftButtonPressed = false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT: {
			CS_KEY_EVENT_PACKET p;
			p.size = sizeof(p);
			p.type = CS_KEY_EVENT;
			p.is_pressed = false;
			p.key = MY_KEY_EVENT::MOUSE_RIGHT;
			NetworkManager::getInstance().sendPacket(&p);
			break;
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		case GLFW_MOUSE_BUTTON_4:
			break;
		case GLFW_MOUSE_BUTTON_5:
			break;
		}
		break;
	}
}

void Scene::processMouseCursor(float xpos, float ypos)
{
	if (leftButtonPressed) {
		camera.processMouseCursor(xpos, ypos);
	}
}

void Scene::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case SC_LOGIN_INFO: {
		auto p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		std::cout << "�α��� ��Ŷ ����, ROOM:ID->[" << int(p->room_id) << ":" << int(p->player_id) << "]\n";
		// �� ID�� ���� ������� ����
		my_id = p->player_id;
		pMyPlayer->setPosition(glm::vec3(p->pos_x, 0.f, p->pos_z));
		pMyPlayer->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		pPlayers[my_id] = pMyPlayer;
		break;
	}
	case SC_LOGOUT: {
		auto p = reinterpret_cast<SC_LOGOUT_PACKET*>(packet);
		std::cout << "�α׾ƿ� ��Ŷ ���� ID:[" << int(p->player_id) << "]\n";
		vkDeviceWaitIdle(fDevice.logicalDevice);	// Vulkan ȣ���ϴ� ������Ʈ ���� ������ ������ ����� �Ѵ�.
		pPlayers[p->player_id] = nullptr;
		break;
	}
	case SC_ADD_PLAYER: {
		auto p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);
		std::cout << "�÷��̾� �߰� ��Ŷ ���� ID:[" << int(p->player_id) << "]\n";
		pPlayers[p->player_id] = std::make_shared<PlayerObject>();		// Todo : ���� Ÿ�Կ� ���� �ٸ��� ����
		pPlayers[p->player_id]->initModel(playerModel[static_cast<int>(p->player_type)], descriptorSetLayout.ssbo);
		pPlayers[p->player_id]->setScale(glm::vec3(1.3f));
		pPlayers[p->player_id]->setAnimationClip(CLIP_IDLE);
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
		if (p->state == PLAYER_STATE::IDLE)
			pPlayers[p->player_id]->setAnimationClip(CLIP_IDLE);
		else if (p->state == PLAYER_STATE::RUN)
			pPlayers[p->player_id]->setAnimationClip(CLIP_RUN);
		else if (p->state == PLAYER_STATE::ATTACK)
			pPlayers[p->player_id]->setAnimationClip(CLIP_ATTACK_KNIFE);
		else
			std::cout << int(p->player_id) << ": STATE ����" << std::endl;
		std::cout << int(p->player_id) << "�� ���°� " << ((p->state == PLAYER_STATE::RUN) ? "RUN" : (p->state == PLAYER_STATE::IDLE) ? "IDLE" : "ATTACK") << "�� ����" << std::endl;
		break;
	}
	case SC_ADD_MONSTER: {
		auto p = reinterpret_cast<SC_ADD_MONSTER_PACKET*>(packet);
		pMonsterObjects.try_emplace(p->monster_id, std::make_shared<GLTFSkinModelObject>());
		pMonsterObjects[p->monster_id]->initModel(mushroomModel, descriptorSetLayout.ssbo);
		pMonsterObjects[p->monster_id]->setPosition({ p->pos_x, 0.f, p->pos_z });
		pMonsterObjects[p->monster_id]->setLook({ p->dir_x, 0.f, p->dir_z });
		std::cout << int(p->monster_id) << ": ���� �߰�" << std::endl;
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
		vkDeviceWaitIdle(fDevice.logicalDevice);	// Vulkan ȣ���ϴ� ������Ʈ ���� ������ ������ ����� �Ѵ�.
		pMonsterObjects.erase(p->monster_id);
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
			std::cout << int(p->monster_id) << ": STATE ����" << std::endl;
			break;
		}
		std::string state_str[5]{ "IDLE", "MOVE", "HIT", "DIE", "ATTACK" };
		std::cout << "���� " << int(p->monster_id) << "�� ���°� "
			<< state_str[static_cast<int>(p->state)] << "�� ����" << std::endl;
		break;
	}
	}
}

PLAYER_TYPE Scene::getPlayerType() const
{
	return player_type;
}

void Scene::createDescriptorSetLayout()
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

void Scene::createGraphicsPipeline()
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
	colorBlendAttachment.blendEnable = VK_TRUE; // ���� Ȱ��ȭ

	// ���� ���� ����
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;				// �ҽ� ����
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;		// ��� ����
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;								// ���� ����

	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;						// �ҽ� ���� �� ����
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;					// ��� ���Ŀ� ������ ���� ����
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;								// ���� ���� ����

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

	// ���� ���� ��ũ���� ���� ����� ��, set�� index�� pSetLayouts�� index�� ������� �Ѵ�.
	std::vector<VkDescriptorSetLayout> setLayout{ 4 };
	setLayout[0] = shadowSetLayout;
	setLayout[1] = descriptorSetLayout.sampler;
	setLayout[2] = descriptorSetLayout.ubo;
	setLayout[3] = descriptorSetLayout.ssbo;										// skinModel������ ���

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(vkf::PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayout.size());	// model������ (0, 1, 2) ���, offscreen�� 0�� X
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

	// skinModel�� pipeline ����
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

	// offscreen ���������� ����
	{
		// �׸��� �����ÿ��� ��Ƽ���ø� Off
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// No blend attachment states (no color attachments used)
		colorBlending.attachmentCount = 0;
		// Disable culling, so all faces contribute to shadows
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;			// �׸��� ���ÿ����� �⺻ OP�� �̰ɷ� �Ǿ��ִ�
		// Enable depth bias
		rasterizer.depthBiasEnable = VK_TRUE;
		// Add depth bias to dynamic state, so we can change it at runtime
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		pipelineInfo.renderPass = renderPass.offscreen;
	}
	// skinModel, ������ pStages�� skinModel Shader�� ����Ǿ� �ִ�
	pipelineInfo.stageCount = 1;		// vertex shader�� ���

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
}
