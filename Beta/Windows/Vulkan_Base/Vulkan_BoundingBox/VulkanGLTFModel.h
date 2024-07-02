#pragma once

#include "VulkanFramework.h"

#include <memory>

namespace tinygltf
{
	class Node;
	class Model;
}

class VulkanGLTFModel
{
public:
	vkf::Device* fDevice = nullptr;

	VkDescriptorPool samplerDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout samplerDescriptorSetLayout = VK_NULL_HANDLE;

	// 실제 모델에서 불러온 버퍼
	std::vector<vkf::Vertex> vertexBuffer;
	std::vector<uint32_t> indexBuffer;

	vkf::MeshBuffer buffer;

	// The following structures roughly represent the glTF scene structure
	// To keep things simple, they only contain those properties that are required for this sample
	struct Node;

	// A primitive contains the data for a single draw call
	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		int32_t materialIndex;
	};

	// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
	struct Mesh {
		std::vector<Primitive> primitives;
	};

	// A node represents an object in the glTF scene graph
	struct Node {
		std::shared_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;
		Mesh mesh;
		glm::mat4 matrix;
	};

	// A glTF material stores information in e.g. the texture that is attached to it and colors
	struct Material {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t baseColorTextureIndex;
	};

	// Contains the texture for a single glTF image
	// Images may be reused by texture objects and are as such separated
	struct Image {
		//vks::Texture2D texture;
		vkf::Texture texture;
		// We also store (and create) a descriptor set that's used to access this texture from the fragment shader
		//VkDescriptorSet descriptorSet;
	};

	// A glTF texture stores a reference to the image and a sampler
	// In this sample, we are only interested in the image
	struct TextureID {
		int32_t imageIndex;
	};

	/*
		Model data
	*/
	std::vector<Image> images;
	std::vector<TextureID> textures;
	std::vector<Material> materials;
	std::vector<std::shared_ptr<Node>> nodes;


public:
	void destroy();

	void loadModel(vkf::Device& fDevice, VkDescriptorSetLayout samplerDescriptorSetLayout, std::string filename);

	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const glm::mat4& worldMatrix = glm::mat4{ 1.f });

private:
	void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const std::shared_ptr<VulkanGLTFModel::Node>& node, const glm::mat4& worldMatrix);

	void loadglTFFile(std::string filename);

	void createSamplerDescriptorPool(uint32_t setCount);
	void loadImages(tinygltf::Model& input);
	void loadTextures(tinygltf::Model& input);
	void loadMaterials(tinygltf::Model& input);
	void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, const std::shared_ptr<VulkanGLTFModel::Node>& parent);


};

