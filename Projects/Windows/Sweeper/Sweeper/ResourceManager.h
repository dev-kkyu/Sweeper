#pragma once

#include "VulkanFramework.h"

// 싱클톤 클래스
class ResourceManager
{
private:
	const vkf::RenderPass* pRenderPass;
	const VkSampleCountFlagBits* pMsaaSamples;

	struct DescriptorSetLayout {
		VkDescriptorSetLayout ubo;
		VkDescriptorSetLayout sampler;
		VkDescriptorSetLayout ssbo;
	} descriptorSetLayout;

	VkPipelineLayout pipelineLayout;		// model 파이프라인은 0, 1, 2번 셋만 사용한다

	// GLTF 모델 draw시 사용할 pipeline 정의
	struct ModelPipeline {
		VkPipeline model;
		VkPipeline skinModel;
	};
	// 공용 pipeline 정의
	struct Pipeline {
		union {
			ModelPipeline sceneOnOff[2];
			struct {
				ModelPipeline scene;
				ModelPipeline offscreen;
			};
		};
		// 사각형 텍스처 draw에 사용하는 pipeline, // Todo : 아직 사용안함
		VkPipeline quad;
	} pipeline;

private:	// 싱글톤
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

public:
	static ResourceManager& getInstance();

public:
	void init(VkDevice logicalDevice, const vkf::RenderPass& renderPass, const VkSampleCountFlagBits& msaaSamples);
	void destroy(VkDevice logicalDevice);

	const DescriptorSetLayout& getDescriptorSetLayout() const;
	VkPipelineLayout getPipelineLayout() const;
	const Pipeline& getPipeline() const;

private:
	void createDescriptorSetLayout(VkDevice logicalDevice);
	void createPipelineLayout(VkDevice logicalDevice);
	void createGraphicsPipeline(VkDevice logicalDevice);

};

