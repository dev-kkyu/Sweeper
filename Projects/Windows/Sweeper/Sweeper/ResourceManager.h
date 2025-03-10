#pragma once

#include "VulkanFramework.h"
#include "VulkanGLTFSkinModel.h"

#include <array>

// 싱클톤 클래스
class ResourceManager
{
private:
	// 여러 Scene에서 같이 사용되는 GameFramework 리소스들
	const vkf::RenderPass* pRenderPass;
	const VkSampleCountFlagBits* pMsaaSamples;
	const VkDescriptorSet* pShadowDescriptorSet;

	// 이후부터는 해당 클래스가 직접 생성하는 공용 리소스들
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
		// 사각형 텍스처 draw에 사용하는 pipeline
		VkPipeline quad;
	} pipeline;

	// GLTF Skin 모델 - 캐릭터
	std::array<VulkanGLTFSkinModel, 4> playerModel;		// 캐릭터 종류는 총 4개이다.

private:	// 싱글톤
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

public:
	static ResourceManager& getInstance();

public:
	void init(vkf::Device& fDevice, const vkf::RenderPass& renderPass, const VkSampleCountFlagBits& msaaSamples, const VkDescriptorSet& shadowDescriptorSet);
	void destroy(VkDevice logicalDevice);

	const vkf::RenderPass& getRenderPass() const;
	VkSampleCountFlagBits getMsaaSamples() const;
	const VkDescriptorSet& getShadowDescriptorSet() const;

	const DescriptorSetLayout& getDescriptorSetLayout() const;
	VkPipelineLayout getPipelineLayout() const;
	const Pipeline& getPipeline() const;

	std::array<VulkanGLTFSkinModel, 4>& getPlayerModel();

private:
	void createDescriptorSetLayout(VkDevice logicalDevice);
	void createPipelineLayout(VkDevice logicalDevice);
	void createGraphicsPipeline(VkDevice logicalDevice);

};

