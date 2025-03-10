#include "ResourceManager.h"

#include <stdexcept>
#include <array>

ResourceManager::ResourceManager()
{
	pRenderPass = nullptr;
	pMsaaSamples = nullptr;
	pShadowDescriptorSet = nullptr;
}

ResourceManager::~ResourceManager()
{
}

ResourceManager& ResourceManager::getInstance()
{
	static ResourceManager instance;
	return instance;
}

void ResourceManager::init(VkDevice logicalDevice, const vkf::RenderPass& renderPass, const VkSampleCountFlagBits& msaaSamples, const VkDescriptorSet& shadowDescriptorSet)
{
	pRenderPass = &renderPass;
	pMsaaSamples = &msaaSamples;
	pShadowDescriptorSet = &shadowDescriptorSet;

	createDescriptorSetLayout(logicalDevice);
	createPipelineLayout(logicalDevice);
	createGraphicsPipeline(logicalDevice);
}

void ResourceManager::destroy(VkDevice logicalDevice)
{
	//vkDestroyPipeline(logicalDevice, pipeline.quad, nullptr);
	vkDestroyPipeline(logicalDevice, pipeline.scene.model, nullptr);
	vkDestroyPipeline(logicalDevice, pipeline.scene.skinModel, nullptr);
	vkDestroyPipeline(logicalDevice, pipeline.offscreen.model, nullptr);
	vkDestroyPipeline(logicalDevice, pipeline.offscreen.skinModel, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.ubo, nullptr);
}

const vkf::RenderPass& ResourceManager::getRenderPass() const
{
	return *pRenderPass;
}

VkSampleCountFlagBits ResourceManager::getMsaaSamples() const
{
	return *pMsaaSamples;
}

const VkDescriptorSet& ResourceManager::getShadowDescriptorSet() const
{
	return *pShadowDescriptorSet;
}

const ResourceManager::DescriptorSetLayout& ResourceManager::getDescriptorSetLayout() const
{
	return descriptorSetLayout;
}

VkPipelineLayout ResourceManager::getPipelineLayout() const
{
	return pipelineLayout;
}

const ResourceManager::Pipeline& ResourceManager::getPipeline() const
{
	return pipeline;
}

void ResourceManager::createDescriptorSetLayout(VkDevice logicalDevice)
{
	// Layout 생성, 해당 프로젝트는 binding은 0으로 통일 및 set 번호로 리소스 구별
	std::array<VkDescriptorSetLayoutBinding, 1> uboLayoutBinding{};
	uboLayoutBinding[0].binding = 0;			// shader의 layout binding, 인덱스와도 맞추기
	uboLayoutBinding[0].descriptorCount = 1;
	uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding[0].pImmutableSamplers = nullptr;
	uboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> samplerLayoutBinding{};
	samplerLayoutBinding[0].binding = 0;
	samplerLayoutBinding[0].descriptorCount = 1;
	samplerLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding[0].pImmutableSamplers = nullptr;
	samplerLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> ssboLayoutBinding{};
	ssboLayoutBinding[0].binding = 0;
	ssboLayoutBinding[0].descriptorCount = 1;
	ssboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ssboLayoutBinding[0].pImmutableSamplers = nullptr;
	ssboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(uboLayoutBinding.size());
	layoutInfo.pBindings = uboLayoutBinding.data();

	if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.ubo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = samplerLayoutBinding.data();
	if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	layoutInfo.pBindings = ssboLayoutBinding.data();
	if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout.ssbo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void ResourceManager::createPipelineLayout(VkDevice logicalDevice)
{
	// 여러 개의 디스크립터 셋을 사용할 때, set의 index를 pSetLayouts의 index와 맞춰줘야 한다.
	std::vector<VkDescriptorSetLayout> setLayout{ 4 };
	setLayout[0] = descriptorSetLayout.sampler;										// 쉐도우맵
	setLayout[1] = descriptorSetLayout.sampler;										// 일반 텍스처 사용시
	setLayout[2] = descriptorSetLayout.ubo;											// 각종 변환 행렬 등
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

	if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void ResourceManager::createGraphicsPipeline(VkDevice logicalDevice)
{
	vkf::Shader modelShader{ logicalDevice, "shaders/model.vert.spv", "shaders/fragment.frag.spv" };

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
	multisampling.rasterizationSamples = *pMsaaSamples;

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
	pipelineInfo.layout = ResourceManager::getInstance().getPipelineLayout();
	pipelineInfo.renderPass = pRenderPass->scene;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.scene.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// skinModel용 pipeline 생성
	vkf::Shader skinModelShader{ logicalDevice, "shaders/skinnedmodel.vert.spv", "shaders/fragment.frag.spv" };
	pipelineInfo.stageCount = static_cast<uint32_t>(skinModelShader.shaderStages.size());
	pipelineInfo.pStages = skinModelShader.shaderStages.data();

	auto skinBindingDescription = vkf::SkinVertex::getBindingDescription();
	auto skinAttributeDescriptions = vkf::SkinVertex::getAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &skinBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(skinAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = skinAttributeDescriptions.data();

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.scene.skinModel) != VK_SUCCESS) {
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

		pipelineInfo.renderPass = pRenderPass->offscreen;
	}
	// skinModel, 위에서 pStages는 skinModel Shader로 연결되어 있다
	pipelineInfo.stageCount = 1;		// vertex shader만 사용

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.offscreen.skinModel) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// Model
	pipelineInfo.pStages = modelShader.shaderStages.data();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.offscreen.model) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
