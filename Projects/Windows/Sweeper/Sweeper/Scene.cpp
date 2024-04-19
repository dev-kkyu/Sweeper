#include "Scene.h"
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "NetworkManager.h"

#define CLIP_IDLE			19
#define CLIP_RUN			24
#define CLIP_ATTACK_KNIFE	10

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	uniformBufferObject.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);

	createSamplerDescriptorPool(2);		// obj�� ����� �ؽ�ó ������ �ҷ��ð�

	// obj�� �ؽ�ó �ε�
	mapBuffer.loadFromObjFile(fDevice, "models/map.obj");
	mapTexture.loadFromFile(fDevice, "textures/map.png", samplerDescriptorPool, descriptorSetLayout.sampler);
	warriorBuffer.loadFromObjFile(fDevice, "models/warrior.obj");
	warriorTexture.loadFromFile(fDevice, "textures/warrior.png", samplerDescriptorPool, descriptorSetLayout.sampler);

	// gltf �� �ε�
	wispModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/wisp.glb");

	// gltf skin�� �ε�
	mushroomModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/blue_mushroom.glb");
	// �÷��̾� �� �ε� (skin model)	// �ϴ��� �ΰ��� �ε��Ѵ�.
	playerModel[0].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Dragoon.glb");
	playerModel[1].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Mage.glb");

	// �� ����
	mapObject = new OBJModelObject;
	mapObject->setBuffer(mapBuffer);
	mapObject->setTexture(mapTexture);

	// ������� ����
	for (int i = 0; i < wispObject.size(); ++i) {
		wispObject[i] = new GLTFModelObject;
		wispObject[i]->setModel(wispModel);
		wispObject[i]->setPosition({ (i - 5) * 5.f - 2.5f, 0.f, 30.f });
		wispObject[i]->rotate(180.f);
	}

	// ���� ����
	for (int i = 0; i < warriorObject.size(); ++i) {
		warriorObject[i] = new OBJModelObject;
		warriorObject[i]->setBuffer(warriorBuffer);
		warriorObject[i]->setTexture(warriorTexture);
		warriorObject[i]->setPosition({ (i - 5) * 5.f, 0.f, 25.f });
		warriorObject[i]->rotate(180.f);
	}

	// �÷��̾� ������ �������� �α��� ���� ���� ������ �̷��ش�.
}

Scene::~Scene()
{
	// �÷��̾�� shared_ptr�̹Ƿ�, ���� ���� X

	for (auto& model : playerModel) {
		model.destroy();
	}

	for (auto& object : wispObject) {
		delete object;
	}

	for (auto& object : warriorObject) {
		delete object;
	}
	warriorTexture.destroy();
	warriorBuffer.destroy();

	pMonsterObjects.clear();
	mushroomModel.destroy();

	delete mapObject;
	mapTexture.destroy();
	mapBuffer.destroy();

	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);

	uniformBufferObject.destroy();

	vkDestroyPipeline(fDevice.logicalDevice, pipeline.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.skinModel, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout.model, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout.skinModel, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ubo, nullptr);
}

void Scene::update(float elapsedTime, uint32_t currentFrame)
{
	camera.update(elapsedTime);

	vkf::UniformBufferObject ubo{};
	ubo.view = camera.getView();
	ubo.projection = camera.getProjection();

	uniformBufferObject.updateUniformBuffer(ubo, currentFrame);

	// �𵨵� ������Ʈ
	mapObject->update(elapsedTime, currentFrame);

	for (auto& m : pMonsterObjects) {
		m.second->update(elapsedTime, currentFrame);
	}

	for (auto& object : warriorObject) {
		object->update(elapsedTime, currentFrame);
	}

	for (auto& object : wispObject) {
		object->update(elapsedTime, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->update(elapsedTime, currentFrame);
	}
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	// non-skinModel
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.model);
	// firstSet�� set�� �����ε���
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.model, 0, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	mapObject->draw(commandBuffer, pipelineLayout.model, currentFrame);
	for (auto& object : warriorObject) {
		object->draw(commandBuffer, pipelineLayout.model, currentFrame);
	}
	for (auto& object : wispObject) {				// ������ alpha �ִ� �� ���� �������� �׷��� �Ѵ�.
		object->draw(commandBuffer, pipelineLayout.model, currentFrame);
	}

	// skinModel
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.skinModel);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.skinModel, 0, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	for (auto& m : pMonsterObjects) {
		m.second->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
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
			if (pMyPlayer)
				pMyPlayer->setStartMousePos(xpos, ypos);	// �¿� ȸ���� (���� ī�޶�� �ѱ� ����)
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
		if (pMyPlayer)
			pMyPlayer->processMouseCursor(xpos, ypos);
		camera.processMouseCursor(xpos, ypos);
	}
}

void Scene::processPacket(unsigned char* packet)
{
	switch (packet[1])
	{
	case SC_LOGIN: {
		auto p = reinterpret_cast<SC_LOGIN_PACKET*>(packet);
		std::cout << "�α��� ��Ŷ ����, ROOM:ID->[" << int(p->room_id) << ":" << int(p->player_id) << "]\n";
		my_id = p->player_id;
		pMyPlayer = std::make_shared<PlayerObject>();
		pMyPlayer->initModel(playerModel[0], descriptorSetLayout.ssbo);
		pMyPlayer->setScale(glm::vec3(1.5f));
		pMyPlayer->setAnimationClip(CLIP_IDLE);	// Idle
		//pMyPlayer->setAnimateSpeed(1.f);	// Todo : �ʿ�� ������ ������ ��
		pMyPlayer->setPosition(glm::vec3(p->pos_x, 0.f, p->pos_z));
		pMyPlayer->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		camera.setPlayer(pMyPlayer);
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
		pPlayers[p->player_id] = std::make_shared<PlayerObject>();
		pPlayers[p->player_id]->initModel(playerModel[1], descriptorSetLayout.ssbo);	// �ϴ� �� ���� 1���𵨷�
		pPlayers[p->player_id]->setScale(glm::vec3(1.5f));
		pPlayers[p->player_id]->setAnimationClip(CLIP_IDLE);
		pPlayers[p->player_id]->setPosition(glm::vec3(p->pos_x, 0.f, p->pos_z));
		pPlayers[p->player_id]->setLook(glm::vec3(p->dir_x, 0.f, p->dir_z));
		break;
	}
	case SC_POSITION: {
		auto p = reinterpret_cast<SC_POSITION_PACKET*>(packet);
		pPlayers[p->player_id]->setPosition(glm::vec3(p->x, p->y, p->z));
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
		break;
	}
	case SC_REMOVE_MONSTER: {
		auto p = reinterpret_cast<SC_REMOVE_MONSTER_PACKET*>(packet);
		break;
	}
	}
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
	vkf::Shader modelShader{ fDevice, "shaders/model.vert.spv", "shaders/model.frag.spv" };

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
	std::vector<VkDescriptorSetLayout> setLayout{ 3 };
	setLayout[0] = descriptorSetLayout.ubo;
	setLayout[1] = descriptorSetLayout.sampler;
	setLayout[2] = descriptorSetLayout.ssbo;									// skinModel������ ���

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(vkf::PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;										// model������ 1�� �ε��������� ���
	pipelineLayoutInfo.pSetLayouts = setLayout.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pipelineLayoutInfo.setLayoutCount = 3;										// skinModel������ 2�� �ε������� ���
	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.skinModel) != VK_SUCCESS) {
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
	pipelineInfo.layout = pipelineLayout.model;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// skinModel�� pipeline ����
	pipelineInfo.layout = pipelineLayout.skinModel;

	vkf::Shader skinModelShader{ fDevice, "shaders/skinnedmodel.vert.spv", "shaders/skinnedmodel.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(skinModelShader.shaderStages.size());
	pipelineInfo.pStages = skinModelShader.shaderStages.data();

	auto skinBindingDescription = vkf::SkinVertex::getBindingDescription();
	auto skinAttributeDescriptions = vkf::SkinVertex::getAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &skinBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(skinAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = skinAttributeDescriptions.data();

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.skinModel) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void Scene::createSamplerDescriptorPool(uint32_t setCount)
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = setCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = setCount;

	if (vkCreateDescriptorPool(fDevice.logicalDevice, &poolInfo, nullptr, &samplerDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

