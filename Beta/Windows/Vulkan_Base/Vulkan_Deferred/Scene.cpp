﻿#include "Scene.h"
#include <stdexcept>

#include <GLFW/glfw3.h>

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass, vkf::OffscreenPass& off)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, offscreenPass { off }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	setDescriptors();

	createSamplerDescriptorPool(2);		// obj에 사용할 텍스처 별도로 불러올것

	uniformBufferObject.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);
	shadowUniformBufferObject.createShadowUniformBufferObjects(fDevice, descriptorSetLayout.shadowUbo);

	// obj와 텍스처 로드
	mapBuffer.loadFromObjFile(fDevice, "models/map.obj");
	mapTexture.loadFromFile(fDevice, "textures/map.png", samplerDescriptorPool, descriptorSetLayout.sampler);
	warriorBuffer.loadFromObjFile(fDevice, "models/warrior.obj");
	warriorTexture.loadFromFile(fDevice, "textures/warrior.png", samplerDescriptorPool, descriptorSetLayout.sampler);

	// gltf skin모델 로드
	mushroomModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/mushroom.glb");
	playerModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/CesiumMan/glTF-Binary/CesiumMan.glb");

	// 맵 생성
	mapObject = new OBJModelObject;
	mapObject->setBuffer(mapBuffer);
	mapObject->setTexture(mapTexture);

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

	// 플레이어 생성
	pPlayer = new PlayerObject;
	pPlayer->initModel(playerModel, descriptorSetLayout.ssbo);
	pPlayer->setPosition({ 1.f, 0.f, 1.f });
	pPlayer->setAnimateSpeed(2.f);

	camera.setPlayer(pPlayer);
}

Scene::~Scene()
{
	delete pPlayer;
	playerModel.destroy();

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

	vkDestroySampler(fDevice.logicalDevice, offscreenPass.depthSampler, nullptr);
	vkDestroyImageView(fDevice.logicalDevice, offscreenPass.shadowImageView, nullptr);
	vkDestroyImage(fDevice.logicalDevice, offscreenPass.shadowImage, nullptr);
	vkFreeMemory(fDevice.logicalDevice, offscreenPass.shadowMemory, nullptr);

	vkDestroyFramebuffer(fDevice.logicalDevice, offscreenPass.frameBuffer, nullptr);

	uniformBufferObject.destroy();
	shadowUniformBufferObject.destroy();

	vkDestroyPipeline(fDevice.logicalDevice, pipeline.model, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.skinModel, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, pipeline.offscreen, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout.model, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout.skinModel, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout.offscreen, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.shadowMap, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.shadowUbo, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, descriptorSetLayout.ubo, nullptr);
}

void Scene::update(float elapsedTime, uint32_t currentFrame)
{
	camera.update(elapsedTime);

	vkf::ShadowUniformBufferObject subo{};
	glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	subo.depthMVP =depthViewMatrix;                   // model * view * project - 모델, 투영 단위행렬

	vkf::UniformBufferObject ubo{};
	ubo.view = camera.getView();
	ubo.proj = camera.getProjection();
	ubo.lightPos = glm::vec4(lightPos, 1.0f);
	ubo.lightSpace = subo.depthMVP;

	uniformBufferObject.updateUniformBuffer(ubo, currentFrame);
	shadowUniformBufferObject.updateShadowUniformBuffer(subo, currentFrame);

	// 모델들 업데이트
	mapObject->update(elapsedTime, currentFrame);

	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i]->update(elapsedTime, currentFrame);
	}

	for (auto& object : warriorObject) {
		object->update(elapsedTime, currentFrame);
	}

	pPlayer->update(elapsedTime, currentFrame);
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

	// skinModel
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.skinModel);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.skinModel, 0, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i]->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
	}

	pPlayer->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);

}

void Scene::offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.offscreen);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.offscreen, 0, 1, &offscreenPass.descriptorSet, 0, nullptr);

	mapObject->draw(commandBuffer, pipelineLayout.model, currentFrame);
	for (auto& object : warriorObject) {
		object->draw(commandBuffer, pipelineLayout.model, currentFrame);
	}
	for (int i = 0; i < mushroomObject.size(); ++i) {
		mushroomObject[i]->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
	}
	pPlayer->draw(commandBuffer, pipelineLayout.skinModel, currentFrame);
}

void Scene::processKeyboard(int key, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_W:
			keyState |= KEY_UP;
			break;
		case GLFW_KEY_S:
			keyState |= KEY_DOWN;
			break;
		case GLFW_KEY_A:
			keyState |= KEY_LEFT;
			break;
		case GLFW_KEY_D:
			keyState |= KEY_RIGHT;
			break;
		}
		pPlayer->processKeyInput(keyState);
		break;
	case GLFW_RELEASE:
		switch (key) {
		case GLFW_KEY_W:
			keyState &= ~KEY_UP;
			break;
		case GLFW_KEY_S:
			keyState &= ~KEY_DOWN;
			break;
		case GLFW_KEY_A:
			keyState &= ~KEY_LEFT;
			break;
		case GLFW_KEY_D:
			keyState &= ~KEY_RIGHT;
			break;
		}
		pPlayer->processKeyInput(keyState);
		break;
	case GLFW_REPEAT:
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
			pPlayer->setStartMousePos(xpos, ypos);
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
		pPlayer->processMouseCursor(xpos, ypos);
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

	VkDescriptorSetLayoutBinding shadowUboLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding shadowMapLayoutBinding{};
	shadowMapLayoutBinding.binding = 0;
	shadowMapLayoutBinding.descriptorCount = 1;
	shadowMapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadowMapLayoutBinding.pImmutableSamplers = nullptr;
	shadowMapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

	layoutInfo.pBindings = &shadowUboLayoutBinding;
	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.shadowUbo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = &shadowMapLayoutBinding;
	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.shadowMap) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = &ssboLayoutBinding;
	if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.ssbo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void Scene::setDescriptors()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(fDevice.logicalDevice, &poolInfo, nullptr, &offscreenPass.descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	VkDescriptorSetAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = offscreenPass.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout.shadowMap;

	if (vkAllocateDescriptorSets(fDevice.logicalDevice, &allocInfo, &offscreenPass.descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	VkDescriptorImageInfo shadowMapDescriptor;
	shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	shadowMapDescriptor.imageView = offscreenPass.shadowImageView;
	shadowMapDescriptor.sampler = offscreenPass.depthSampler;

	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = offscreenPass.descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &shadowMapDescriptor;

	vkUpdateDescriptorSets(fDevice.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
	colorBlendAttachment.blendEnable = VK_FALSE;

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
	setLayout[0] = descriptorSetLayout.ubo;
	setLayout[1] = descriptorSetLayout.sampler;
	setLayout[2] = descriptorSetLayout.shadowMap;
	setLayout[3] = descriptorSetLayout.ssbo;									// skinModel에서만 사용

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(vkf::PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 3;										// model에서는 2번 인덱스까지만 사용
	pipelineLayoutInfo.pSetLayouts = setLayout.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pipelineLayoutInfo.setLayoutCount = 4;										// skinModel에서는 3번 인덱스까지 사용
	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.skinModel) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.shadowUbo;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout.offscreen) != VK_SUCCESS) {
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

	vkf::Shader shadowMapShader{ fDevice, "shaders/offscreen.vert.spv", "shaders/model.frag.spv" };
	pipelineInfo.stageCount = 1;
	pipelineInfo.pStages = &shadowMapShader.shaderStages[0];
	colorBlending.attachmentCount = 0;
	rasterizer.depthBiasEnable = VK_TRUE;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	pipelineInfo.renderPass = offscreenPass.renderPass;
	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.offscreen) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void Scene::createSamplerDescriptorPool(uint32_t setCount)
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
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