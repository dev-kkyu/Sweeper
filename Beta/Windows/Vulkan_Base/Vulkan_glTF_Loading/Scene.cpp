#include "Scene.h"
#include <stdexcept>

#include <GLFW/glfw3.h>

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, VkRenderPass& renderPass)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	createUniformBuffers();
	createUboDescriptorPool();
	createUboDescriptorSets();

	createSamplerDescriptorPool(2);

	plainBuffer.loadFromObjFile(fDevice, "models/tile.obj");
	plainTexture.loadFromFile(fDevice, "textures/tile.jpg", samplerDescriptorPool, samplerDescriptorSetLayout);
	boxBuffer.loadFromObjFile(fDevice, "models/box.obj");
	boxTexture.loadFromFile(fDevice, "textures/wood.jpg", samplerDescriptorPool, samplerDescriptorSetLayout);

	plainObject = new GameObjectObj;
	plainObject->setBuffer(plainBuffer);
	plainObject->setTexture(plainTexture);

	pPlayer = new PlayerObject;
	pPlayer->setBuffer(boxBuffer);
	pPlayer->setTexture(boxTexture);
	pPlayer->setPosition({ 1.f, 0.f, 1.f });
	pPlayer->setLook({ 0.f, 0.f, -1.f });

	camera.setPlayer(pPlayer);

	sampleModel[0].loadModel(fDevice, samplerDescriptorSetLayout, "models/FlightHelmet/glTF/FlightHelmet.gltf");
	sampleModel[1].loadModel(fDevice, samplerDescriptorSetLayout, "models/CesiumMan/glTF-Binary/CesiumMan.glb");
	//sampleModel[1].loadModel(fDevice, samplerDescriptorSetLayout, "models/CesiumMan/glTF/CesiumMan.gltf");
	//sampleModel[1].loadModel(fDevice, samplerDescriptorSetLayout, "models/CesiumMan/glTF-Embedded/CesiumMan.gltf");

	sampleModelObject[0] = new GameObjectglTF;
	sampleModelObject[1] = new GameObjectglTF;

	sampleModelObject[0]->setModel(sampleModel[0]);
	sampleModelObject[0]->setPosition({ 0.f, 2.f, 0.f });
	sampleModelObject[0]->setScale(glm::vec3{ 3.f });
	sampleModelObject[1]->setModel(sampleModel[1]);
	sampleModelObject[1]->setPosition({ 2.f, 0.f, 0.f });
	sampleModelObject[1]->setScale(glm::vec3{ 2.f });
}

Scene::~Scene()
{
	delete sampleModelObject[0];
	delete sampleModelObject[1];
	sampleModel[0].destroy();
	sampleModel[1].destroy();

	delete pPlayer;
	boxTexture.destroy();
	boxBuffer.destroy();

	delete plainObject;
	plainTexture.destroy();
	plainBuffer.destroy();

	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);

	vkDestroyDescriptorPool(fDevice.logicalDevice, uboDescriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(fDevice.logicalDevice, uniformBuffers[i], nullptr);
		vkFreeMemory(fDevice.logicalDevice, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyPipeline(fDevice.logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(fDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, samplerDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(fDevice.logicalDevice, uboDescriptorSetLayout, nullptr);
}

void Scene::update(float elapsedTime, uint32_t currentFrame)
{
	camera.update(elapsedTime);

	UniformBufferObject ubo{};
	ubo.view = camera.getView();
	ubo.proj = camera.getProjection();

	memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));


	plainObject->update(elapsedTime);

	pPlayer->update(elapsedTime);

	sampleModelObject[0]->update(elapsedTime);
	sampleModelObject[1]->update(elapsedTime);
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	// firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &uboDescriptorSets[currentFrame], 0, nullptr);

	plainObject->draw(commandBuffer, pipelineLayout);
	pPlayer->draw(commandBuffer, pipelineLayout);
	sampleModelObject[0]->draw(commandBuffer, pipelineLayout);
	sampleModelObject[1]->draw(commandBuffer, pipelineLayout);
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
	if (leftButtonPressed){
		pPlayer->processMouseCursor(xpos, ypos);
	}
}

void Scene::createDescriptorSetLayout()
{
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &uboDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &samplerLayoutBinding;

		if (vkCreateDescriptorSetLayout(fDevice.logicalDevice, &layoutInfo, nullptr, &samplerDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
}

void Scene::createGraphicsPipeline()
{
	auto vertShaderCode = vkf::readFile("shaders/vert.spv");
	auto fragShaderCode = vkf::readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = vkf::createShaderModule(fDevice, vertShaderCode);
	VkShaderModule fragShaderModule = vkf::createShaderModule(fDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = vkf::Vertex::getBindingDescription();
	auto attributeDescriptions = vkf::Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
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
	std::vector<VkDescriptorSetLayout> descriptorSetLayout{ 2 };
	descriptorSetLayout[0] = uboDescriptorSetLayout;
	descriptorSetLayout[1] = samplerDescriptorSetLayout;

	// push constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(fDevice.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(fDevice.logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(fDevice.logicalDevice, vertShaderModule, nullptr);
}

void Scene::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

		vkMapMemory(fDevice.logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}

void Scene::createUboDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(fDevice.logicalDevice, &poolInfo, nullptr, &uboDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Scene::createUboDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, uboDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = uboDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(fDevice.logicalDevice, &allocInfo, uboDescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = uboDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(fDevice.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

