#pragma once

#include "VulkanFramework.h"

class VulkanglTFModel
{
public:
	vkf::Device* fDevice = nullptr;

	VkDescriptorPool samplerDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout samplerDescriptorSetLayout = VK_NULL_HANDLE;

	vkf::Buffer buffer;
	uint32_t indexCount = 0;

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
		Node* parent;
		std::vector<Node*> children;
		Mesh mesh;
		glm::mat4 matrix;
		~Node() {
			for (auto& child : children) {
				delete child;
			}
		}
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
	std::vector<Node*> nodes;


public:
	void destroy();

	void loadModel(vkf::Device& fDevice, VkDescriptorSetLayout samplerDescriptorSetLayout, std::string filename);

	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::mat4 parentMatrix = glm::mat4(1.f));

private:
	void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VulkanglTFModel::Node* node, const glm::mat4& parentMatrix);

	void loadglTFFile(std::string filename);

	void createSamplerDescriptorPool(uint32_t setCount);
	void loadImages(void* pinput);
	void loadTextures(void* pinput);
	void loadMaterials(void* pinput);
	void loadNode(const void* pinputNode, const void* pinput, VulkanglTFModel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<vkf::Vertex>& vertexBuffer);


};

