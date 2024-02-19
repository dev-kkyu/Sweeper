#pragma once

#define MAX_FRAMES_IN_FLIGHT 2

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <string>
#include <array>
#include <vector>
#include <unordered_map>

namespace vkf
{
	struct Device
	{
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkCommandPool commandPool;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

	class Buffer
	{
	private:
		vkf::Device* fDevice = nullptr;

	public:
		std::vector<vkf::Vertex> vertices;
		std::vector<uint32_t> indices;

		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

	public:
		void loadFromObjFile(vkf::Device& fDevice, std::string filename);
		void destroy();

	private:
		void loadObjModel(std::string filename);
		void createVertexBuffer();
		void createIndexBuffer();

	};

	class Texture
	{
		vkf::Device* fDevice = nullptr;
		uint32_t mipLevels;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

	public:
		VkDescriptorSet samplerDescriptorSet;

	public:
		void loadFromFile(vkf::Device& fDevice, VkDescriptorPool samplerDescriptorPool, VkDescriptorSetLayout samplerDescriptorSetLayout, std::string filename);
		void destroy();

	private:
		void createTextureImage(std::string filename);
		void createTextureImageView();
		void createTextureSampler();
		void createSamplerDescriptorSets(VkDescriptorPool samplerDescriptorPool, VkDescriptorSetLayout samplerDescriptorSetLayout);

		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	};

	VkImageView createImageView(Device& fDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImage(Device& fDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(Device& fDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(Device& fDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void createBuffer(Device& fDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands(Device& fDevice);
	void endSingleTimeCommands(Device& fDevice, VkCommandBuffer commandBuffer);

	void copyBuffer(Device& fDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(Device& fDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(Device& fDevice, const std::vector<char>& code);
}

namespace std {
	template<> struct hash<vkf::Vertex> {
		size_t operator()(vkf::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
