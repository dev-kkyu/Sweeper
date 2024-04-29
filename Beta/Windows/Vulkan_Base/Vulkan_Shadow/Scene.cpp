#include "Scene.h"
#include <stdexcept>

#include <GLFW/glfw3.h>

Scene::Scene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkDescriptorSetLayout& shadowSetLayout)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, shadowSetLayout{ shadowSetLayout }
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	uniformBufferObject.scene.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);
	uniformBufferObject.offscreen.createUniformBufferObjects(fDevice, descriptorSetLayout.ubo);

	createSamplerDescriptorPool(2);

	plainBuffer.loadFromObjFile(fDevice, "models/tile.obj");
	plainTexture.loadFromFile(fDevice, "textures/tile.jpg", samplerDescriptorPool, descriptorSetLayout.sampler);
	boxBuffer.loadFromObjFile(fDevice, "models/box.obj");
	boxTexture.loadFromFile(fDevice, "textures/wood.jpg", samplerDescriptorPool, descriptorSetLayout.sampler);

	plainObject = new OBJModelObject;
	plainObject->setBuffer(plainBuffer);
	plainObject->setTexture(plainTexture);

	pPlayer = new PlayerObject;
	pPlayer->setBuffer(boxBuffer);
	pPlayer->setTexture(boxTexture);
	pPlayer->setPosition({ 1.f, 0.f, 1.f });
	pPlayer->setLook({ 0.f, 0.f, -1.f });

	camera.setPlayer(pPlayer);

	gltfModel.loadModel(fDevice, descriptorSetLayout.sampler, "models/deer.gltf");

	skinModel[0].loadModel(fDevice, descriptorSetLayout.sampler, "models/CesiumMan/glTF-Binary/CesiumMan.glb");
	//skinModel[0].loadModel(fDevice, descriptorSetLayout.sampler, "models/CesiumMan/glTF/CesiumMan.gltf");
	//skinModel[0].loadModel(fDevice, descriptorSetLayout.sampler, "models/CesiumMan/glTF-Embedded/CesiumMan.gltf");
	skinModel[1].loadModel(fDevice, descriptorSetLayout.sampler, "models/mushroom.glb");

	gltfModelObject = new GLTFModelObject;
	gltfModelObject->setModel(gltfModel);
	gltfModelObject->setPosition({ 0.f, 1.5f, 0.f });
	gltfModelObject->setScale(glm::vec3{ 1.f });

	skinModelObject[0] = new GLTFSkinModelObject;
	skinModelObject[0]->initModel(skinModel[0], descriptorSetLayout.ssbo);
	skinModelObject[0]->setPosition({ 2.f, 0.f, 0.f });
	skinModelObject[0]->setScale(glm::vec3{ 2.f });
	skinModelObject[0]->setAnimateSpeed(1.5f);
	skinModelObject[1] = new GLTFSkinModelObject;
	skinModelObject[1]->initModel(skinModel[1], descriptorSetLayout.ssbo);
	skinModelObject[1]->setPosition({ -2.f, 0.f, 0.f });
	skinModelObject[1]->setScale(glm::vec3{ 2.f });
}

Scene::~Scene()
{
	delete skinModelObject[0];
	delete skinModelObject[1];
	skinModel[0].destroy();
	skinModel[1].destroy();

	delete gltfModelObject;
	gltfModel.destroy();

	delete pPlayer;
	boxTexture.destroy();
	boxBuffer.destroy();

	delete plainObject;
	plainTexture.destroy();
	plainBuffer.destroy();

	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);

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
	// 카메라 업데이트
	camera.update(elapsedTime);

	// ubo 업데이트
	vkf::UniformBufferObject ubo{};
	ubo.view = camera.getView();
	ubo.projection = camera.getProjection();
	ubo.lightPos = lightPos;

	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProjection = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 100.f);
	ubo.lightSpaceMatrix = lightProjection * lightView;

	uniformBufferObject.scene.updateUniformBuffer(ubo, currentFrame);

	ubo.view = lightView;
	ubo.projection = lightProjection;
	uniformBufferObject.offscreen.updateUniformBuffer(ubo, currentFrame);

	// 오브젝트 업데이트
	plainObject->update(elapsedTime, currentFrame);

	pPlayer->update(elapsedTime, currentFrame);

	gltfModelObject->update(elapsedTime, currentFrame);

	skinModelObject[0]->update(elapsedTime, currentFrame);
	skinModelObject[1]->update(elapsedTime, currentFrame);
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isOffscreen)
{
	// if문을 쓰지 않기 위한 코드..
	Pipeline::Type* lineType[2]{ &pipeline.scene, &pipeline.offscreen };
	vkf::BufferObject* uboType[2]{ &uniformBufferObject.scene, &uniformBufferObject.offscreen };
	int idx = !!static_cast<int>(isOffscreen);
	Pipeline::Type& line = *lineType[idx];
	vkf::BufferObject& ubo = *uboType[idx];

	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &ubo.descriptorSets[currentFrame], 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, line.model);

	plainObject->draw(commandBuffer, pipelineLayout, currentFrame);
	pPlayer->draw(commandBuffer, pipelineLayout, currentFrame);
	gltfModelObject->draw(commandBuffer, pipelineLayout, currentFrame);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, line.skinModel);
	// ubo는 공용이기 때문에, 다시 bind 하지 않는다
	skinModelObject[0]->draw(commandBuffer, pipelineLayout, currentFrame);
	skinModelObject[1]->draw(commandBuffer, pipelineLayout, currentFrame);
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
			camera.setStartMousePos(xpos, ypos);			// 위아래 회전용 (위아래 회전은 플레이어 방향에 영향을 주지 않는다)
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
		camera.processMouseCursor(xpos, ypos);
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
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayout.size());	// model에서는 1번 인덱스까지만 사용 (2개)
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

