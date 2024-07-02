#pragma once

#include "VulkanFramework.h"

#include <memory>

#include <glm/gtc/quaternion.hpp>		// glm::quat

namespace tinygltf
{
	class Node;
	class Model;
}

class VulkanGLTFSkinModel
{
public:
	vkf::Device* fDevice = nullptr;

	std::shared_ptr<tinygltf::Model> glTFInput;

	VkDescriptorPool samplerDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout samplerDescriptorSetLayout = VK_NULL_HANDLE;

	vkf::MeshBuffer buffer;

	struct Material
	{
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t  baseColorTextureIndex;
	};

	struct Image
	{
		vkf::Texture texture;
	};

	struct TextureID
	{
		int32_t imageIndex;
	};

	struct Primitive
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		int32_t  materialIndex;
	};

	struct Mesh
	{
		std::vector<Primitive> primitives;
	};


	// ���⼭���� ������Ʈ���� ������ ��� �־�� ��.

	struct Node
	{
		std::shared_ptr<Node>				parent;
		uint32_t							index;
		std::vector<std::shared_ptr<Node>>	children;
		Mesh								mesh;
		glm::vec3							translation{};
		glm::vec3							scale{ 1.0f };
		glm::quat							rotation{};
		int32_t								skin = -1;
		glm::mat4							matrix;
		glm::mat4							getLocalMatrix();
	};

	struct Skin
	{
		std::string							name;
		std::shared_ptr<Node>				skeletonRoot;
		std::vector<glm::mat4>				inverseBindMatrices;
		std::vector<std::shared_ptr<Node>>	joints;
		vkf::BufferObject					ssbo;					// �Ҹ�� �� destroy ����� ��
	};

	struct AnimationSampler
	{
		std::string							interpolation;
		std::vector<float>					inputs;
		std::vector<glm::vec4>				outputsVec4;
	};

	struct AnimationChannel
	{
		std::string							path;
		std::shared_ptr<Node>				node;
		uint32_t							samplerIndex;
	};

	struct Animation
	{
		std::string							name;
		std::vector<AnimationSampler>		samplers;
		std::vector<AnimationChannel>		channels;
		float								start = std::numeric_limits<float>::max();
		float								end = std::numeric_limits<float>::min();
		float								currentTime = 0.0f;
	};

	std::vector<Image>		images;
	std::vector<TextureID>	textures;
	std::vector<Material>	materials;

	// ������Ʈ���� ���������� �߰� �ε��ؾ� �ϴ� ��.
	/*
		std::vector<std::shared_ptr<Node>>	nodes;
		std::vector<Skin>					skins;
		std::vector<Animation>				animations;

		uint32_t activeAnimation = 0;
	*/

public:
	void destroy();

	void loadModel(vkf::Device& fDevice, VkDescriptorSetLayout samplerDescriptorSetLayout, std::string filename);

	void bindBuffers(VkCommandBuffer commandBuffer);

private:

	void loadglTFFile(std::string filename);

	void createSamplerDescriptorPool(uint32_t setCount);
	void loadImages();
	void loadTextures();
	void loadMaterials();
	void loadMeshes(const tinygltf::Node& inputNode, std::vector<vkf::SkinVertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer);
	// MeshBuffer�� ���� ������ ���, Node ������ �� ������Ʈ���� ��´�. �ִϸ��̼��� �����̱� ������, Node�� �����̾�� �Ѵ�.

};

