#include "ResourceManager.h"

#include <stdexcept>
#include <array>

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

ResourceManager& ResourceManager::getInstance()
{
	static ResourceManager instance;
	return instance;
}

void ResourceManager::init(VkDevice logicalDevice)
{
	createDescriptorSetLayout(logicalDevice);
	createPipelineLayout(logicalDevice);
}

void ResourceManager::destroy(VkDevice logicalDevice)
{
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.ssbo, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.sampler, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout.ubo, nullptr);
}

const ResourceManager::DescriptorSetLayout& ResourceManager::getDescriptorSetLayout() const
{
	return descriptorSetLayout;
}

VkPipelineLayout ResourceManager::getPipelineLayout() const
{
	return pipelineLayout;
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
