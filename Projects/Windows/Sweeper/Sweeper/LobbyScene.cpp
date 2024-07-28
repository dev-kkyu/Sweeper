#include "LobbyScene.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

LobbyScene::LobbyScene(vkf::Device& fDevice, VkSampleCountFlagBits& msaaSamples, vkf::RenderPass& renderPass, VkExtent2D& framebufferExtent,
	std::array<VulkanGLTFSkinModel, 4>& playerModel,
	VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout ssboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout,
	VkDescriptorSet shadowSet, VkPipelineLayout pipelineLayout, VkPipeline modelPipeline, VkPipeline skinModelPipeline)
	: fDevice{ fDevice }, msaaSamples{ msaaSamples }, renderPass{ renderPass }, framebufferExtent{ framebufferExtent }, shadowSet{ shadowSet },
	pipelineLayout{ pipelineLayout }, modelPipeline{ modelPipeline }, skinModelPipeline{ skinModelPipeline }
{
	createGraphicsPipeline();
	createSamplerDescriptorPool(5);		// 텍스처 5개

	uniformBufferObject.createUniformBufferObjects(fDevice, uboDescriptorSetLayout);
	offscreenUniformBufferObject.createUniformBufferObjects(fDevice, uboDescriptorSetLayout);

	button[static_cast<char>(PLAYER_TYPE::WARRIOR)].loadFromFile(fDevice, "models/Textures/Button/warrior.png", samplerDescriptorPool, samplerDescriptorSetLayout);
	button[static_cast<char>(PLAYER_TYPE::ARCHER)].loadFromFile(fDevice, "models/Textures/Button/archer.png", samplerDescriptorPool, samplerDescriptorSetLayout);
	button[static_cast<char>(PLAYER_TYPE::MAGE)].loadFromFile(fDevice, "models/Textures/Button/mage.png", samplerDescriptorPool, samplerDescriptorSetLayout);
	button[static_cast<char>(PLAYER_TYPE::HEALER)].loadFromFile(fDevice, "models/Textures/Button/healer.png", samplerDescriptorPool, samplerDescriptorSetLayout);
	startButton.loadFromFile(fDevice, "models/Textures/Button/gamestart.png", samplerDescriptorPool, samplerDescriptorSetLayout);

	podiumModel.loadModel(fDevice, samplerDescriptorSetLayout, "models/Character/Podium.glb");
	podiumObject.setModel(podiumModel);

	// 고정된 조명
	lightPos = glm::vec3(3.f, 5.f, 3.f);

	// offscreen ubo 값 고정
	vkf::UniformBufferObject ubo{};
	ubo.lightPos = lightPos;						// 안씀
	ubo.view = glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 15.f);
	ubo.lightSpace = ubo.projection * ubo.view;		// 안씀
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		offscreenUniformBufferObject.updateUniformBuffer(ubo, i);
	}

	float xPosition = 1.125f;
	podiumObject.setPosition(glm::vec3(xPosition, 0.f, 0.f));
	for (int i = 0; i < 4; ++i) {
		playerObjects[i].initModel(playerModel[i], ssboDescriptorSetLayout);
		playerObjects[i].setAnimationClip(23);
		playerObjects[i].setPosition(glm::vec3(xPosition, 0.f, 0.f));
	}

	isEnd = false;
	selPlayerType = PLAYER_TYPE::WARRIOR;
}

LobbyScene::~LobbyScene()
{
	podiumModel.destroy();

	startButton.destroy();
	for (auto& texture : button) {
		texture.destroy();
	}

	offscreenUniformBufferObject.destroy();
	uniformBufferObject.destroy();

	vkDestroyDescriptorPool(fDevice.logicalDevice, samplerDescriptorPool, nullptr);
	vkDestroyPipeline(fDevice.logicalDevice, buttonPipeline, nullptr);
}

void LobbyScene::update(float elapsedTime, uint32_t currentFrame)
{
	// scene ubo 업데이트
	vkf::UniformBufferObject ubo{};
	ubo.lightPos = lightPos;
	ubo.view = glm::lookAt(glm::vec3(0.f, 1.375f, 4.125f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.projection = glm::perspective(glm::radians(45.f), float(framebufferExtent.width) / float(framebufferExtent.height), 1.f, 100.f);		// 종횡비에 알맞게
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 15.f);
	ubo.lightSpace = lightProj * lightView;
	uniformBufferObject.updateUniformBuffer(ubo, currentFrame);

	for (int i = 0; i < 4; ++i) {
		playerObjects[i].update(elapsedTime, currentFrame);
	}
}

void LobbyScene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	// shadow map bind
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &shadowSet, 0, nullptr);
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &uniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skinModelPipeline);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);

	// UI 그려주기
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, buttonPipeline);
	// 플레이어 버튼
	for (int i = 0; i < 4; ++i) {
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &button[i].samplerDescriptorSet, 0, nullptr);
		glm::mat4 matrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.425f, 0.675f - i * 0.3f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height)), 0.08125f, 1.f));
		if (static_cast<int>(selPlayerType) != i)
			matrix[3][3] = 0.1f;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
	// 게임 시작 버튼
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &startButton.samplerDescriptorSet, 0, nullptr);
	glm::mat4 matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.625f, 0.f))
		* glm::scale(glm::mat4(1.f), glm::vec3(0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height)), 0.0875f, 1.f));
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &matrix);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void LobbyScene::offscreenDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame, VkPipeline offscreenModelPipeline, VkPipeline offscreenSkinModelPipeline)
{
	// UBO 바인드, firstSet은 set의 시작인덱스
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &offscreenUniformBufferObject.descriptorSets[currentFrame], 0, nullptr);

	// 단상 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenModelPipeline);
	podiumObject.draw(commandBuffer, pipelineLayout, currentFrame);

	// 플레이어 띄워주기 - offscreen
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenSkinModelPipeline);
	playerObjects[static_cast<char>(selPlayerType)].draw(commandBuffer, pipelineLayout, currentFrame);
}

void LobbyScene::processKeyboard(int key, int action, int mods)
{
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_0:
		case GLFW_KEY_KP_0:
			selPlayerType = PLAYER_TYPE::WARRIOR;
			break;
		case GLFW_KEY_1:
		case GLFW_KEY_KP_1:
			selPlayerType = PLAYER_TYPE::ARCHER;
			break;
		case GLFW_KEY_2:
		case GLFW_KEY_KP_2:
			selPlayerType = PLAYER_TYPE::MAGE;
			break;
		case GLFW_KEY_3:
		case GLFW_KEY_KP_3:
			selPlayerType = PLAYER_TYPE::HEALER;
			break;
		case GLFW_KEY_ENTER:
		case GLFW_KEY_KP_ENTER:
			isEnd = true;
			break;
		}
		break;
	}
}

void LobbyScene::processMouseButton(int button, int action, int mods, float xpos, float ypos)
{
	switch (action) {
	case GLFW_RELEASE:
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			// 플레이어 선택
			for (int i = 0; i < 4; ++i) {
				// 그린 곳의 위치와 같게한다
				float bLeft = -1.f * (0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height))) - 0.425f;
				float bRight = 1.f * (0.325f / (float(framebufferExtent.width) / float(framebufferExtent.height))) - 0.425f;
				float bTop = 1.f * 0.08125f + (0.675f - i * 0.3f);
				float bBottom = -1.f * 0.08125f + (0.675f - i * 0.3f);
				if (xpos > bLeft and xpos < bRight and ypos < bTop and ypos > bBottom) {	// 선택
					selPlayerType = static_cast<PLAYER_TYPE>(i);
					break;
				}
			}
			// 게임 시작 버튼 선택
			{
				float bLeft = -1.f * 0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height));
				float bRight = 1.f * 0.35f / (float(framebufferExtent.width) / float(framebufferExtent.height));
				float bTop = 1.f * 0.0875f - 0.625f;
				float bBottom = -1.f * 0.0875f - 0.625f;
				if (xpos > bLeft and xpos < bRight and ypos < bTop and ypos > bBottom) {	// 선택
					isEnd = true;
					break;
				}
			}
			break;
		}
		break;
	}
}

PLAYER_TYPE LobbyScene::getPlayerType() const
{
	return selPlayerType;
}

bool LobbyScene::getIsEnd() const
{
	return isEnd;
}

void LobbyScene::createGraphicsPipeline()
{
	vkf::Shader buttonShader{ fDevice, "shaders/lobbybutton.vert.spv", "shaders/lobbybutton.frag.spv" };

	// input이 없는 shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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

	// 깊이검사 해제
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
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

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(buttonShader.shaderStages.size());
	pipelineInfo.pStages = buttonShader.shaderStages.data();
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

	if (vkCreateGraphicsPipelines(fDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &buttonPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void LobbyScene::createSamplerDescriptorPool(uint32_t setCount)
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
