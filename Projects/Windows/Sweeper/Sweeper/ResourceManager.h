#pragma once

#include "VulkanFramework.h"

// 싱클톤 클래스
class ResourceManager
{
private:
	struct DescriptorSetLayout {
		VkDescriptorSetLayout ubo;
		VkDescriptorSetLayout sampler;
		VkDescriptorSetLayout ssbo;
	} descriptorSetLayout;

	VkPipelineLayout pipelineLayout;		// model 파이프라인은 0, 1, 2번 셋만 사용한다

private:	// 싱글톤
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

public:
	static ResourceManager& getInstance();

public:
	void init(VkDevice logicalDevice);
	void destroy(VkDevice logicalDevice);

	const DescriptorSetLayout& getDescriptorSetLayout() const;
	VkPipelineLayout getPipelineLayout() const;

private:
	void createDescriptorSetLayout(VkDevice logicalDevice);
	void createPipelineLayout(VkDevice logicalDevice);

};

