#include "Scene.h"
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "NetworkManager.h"

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	uniformBufferObject.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);

	createSamplerDescriptorPool(2);		// obj에 사용할 텍스처 별도로 불러올것

	// obj와 텍스처 로드
	mapBuffer.loadFromObjFile(fDevice, "models/map.obj");
	mapTexture.loadFromFile(fDevice, "textures/map.png", samplerDescriptorPool, descriptorSetLayout.sampler);
	warriorBuffer.loadFromObjFile(fDevice, "models/warrior.obj");
	warriorTexture.loadFromFile(fDevice, "textures/warrior.png", samplerDescriptorPool, descriptorSetLayout.sampler);

	// gltf 모델 로드
	wispModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/wisp.glb");

	// gltf skin모델 로드
	mushroomModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/mushroom.glb");
	// 플레이어 모델 로드 (skin model)	// 일단은 두개만 로드한다.
	playerModel[0].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Dragoon.glb");
	playerModel[1].loadModel(fDevice, descriptorSetLayout.sampler, "models/Character/Mage.glb");

	// 맵 생성
	mapObject = new OBJModelObject;
	mapObject->setBuffer(mapBuffer);
	mapObject->setTexture(mapTexture);

	// 도깨비불 생성
	for (int i = 0; i < wispObject.size(); ++i) {
		wispObject[i] = new GLTFModelObject;
		wispObject[i]->setModel(wispModel);
		wispObject[i]->setPosition({ (i - 5) * 5.f - 2.5f, 0.f, 30.f });
		wispObject[i]->rotate(180.f);
	}

	// 버섯 생성
	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i] = new GLTFSkinModelObject;
		mushroomObject[i]->initModel(mushroomModel, descriptorSetLayout.ssbo);
		int x = i / 10 - 5;
		int z = i % 10 - 5;
		mushroomObject[i]->setPosition({ x * 5.f, 0.f, z * 5.f });
		mushroomObject[i]->setAnimateSpeed(float(rand()) / RAND_MAX + 0.5f);
	}

	// 전사 생성
	for (int i = 0; i < warriorObject.size(); ++i) {
		warriorObject[i] = new OBJModelObject;
		warriorObject[i]->setBuffer(warriorBuffer);
		warriorObject[i]->setTexture(warriorTexture);
		warriorObject[i]->setPosition({ (i - 5) * 5.f, 0.f, 25.f });
		warriorObject[i]->rotate(180.f);
	}

	// 플레이어 생성은 서버에서 로그인 정보 받을 때까지 미뤄준다.
}

Scene::~Scene()
{
	// 플레이어는 shared_ptr이므로, 따로 삭제 X

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

	for (auto& object : mushroomObject) {
		delete object;
	}
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
	ubo.proj = camera.getProjection();

	uniformBufferObject.updateUniformBuffer(ubo, currentFrame);

	// 모델들 업데이트
	mapObject->update(elapsedTime, currentFrame);

	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i]->update(elapsedTime, currentFrame);
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
	// firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.model, 0, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	mapObject->draw(commandBuffer, pipelineLayout.model, currentFrame);
	for (auto& object : warriorObject) {
		object->draw(commandBuffer, pipelineLayout.model, currentFrame);
	}
	for (auto& object : wispObject) {				// 원래는 alpha 있는 것 따로 마지막에 그려야 한다.
		object->draw(commandBuffer, pipelineLayout.model, currentFrame);
	}

	// skinModel
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.skinModel);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.skinModel, 0, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i]->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
	}

	for (auto& player : pPlayers) {
		if (player)
			player->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
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
				pMyPlayer->setStartMousePos(xpos, ypos);
			camera.setStartMousePos(xpos, ypos);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
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
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
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
		std::cout << "로그인 패킷 수신, ROOM:ID->[" << int(p->room_id) << ":" << int(p->player_id) << "]\n";
		my_id = p->player_id;
		pMyPlayer = std::make_shared<PlayerObject>();
		pMyPlayer->initModel(playerModel[0], descriptorSetLayout.ssbo);
		pMyPlayer->setScale(glm::vec3(1.5f));
		pMyPlayer->setAnimationClip(19);	// Idle
		//pMyPlayer->setAnimateSpeed(1.f);	// Todo : 필요시 적절히 조절할 것
		camera.setPlayer(pMyPlayer);
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
		pPlayers[p->player_id] = std::make_shared<PlayerObject>();
		pPlayers[p->player_id]->initModel(playerModel[1], descriptorSetLayout.ssbo);	// 일단 나 제외 1번모델로
		pPlayers[p->player_id]->setScale(glm::vec3(1.5f));
		pPlayers[p->player_id]->setAnimationClip(19);
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
			pPlayers[p->player_id]->setAnimationClip(19);
		else if (p->state == PLAYER_STATE::RUN)
			pPlayers[p->player_id]->setAnimationClip(24);
		std::cout << int(p->player_id) << "의 상태가 " << ((p->state == PLAYER_STATE::RUN) ? "RUN" : "IDLE") << "로 변경" << std::endl;
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
	std::vector<VkDescriptorSetLayout> setLayout{ 3 };
	setLayout[0] = descriptorSetLayout.ubo;
	setLayout[1] = descriptorSetLayout.sampler;
	setLayout[2] = descriptorSetLayout.ssbo;									// skinModel에서만 사용

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(vkf::PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;										// model에서는 1번 인덱스까지만 사용
	pipelineLayoutInfo.pSetLayouts = setLayout.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pipelineLayoutInfo.setLayoutCount = 3;										// skinModel에서는 2번 인덱스까지 사용
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

	// skinModel용 pipeline 생성
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

