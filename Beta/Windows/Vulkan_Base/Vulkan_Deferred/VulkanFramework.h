#pragma once

#define MAX_FRAMES_IN_FLIGHT 2

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <array>
#include <vector>

namespace vkf
{
	struct Device
	{
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkCommandPool commandPool;

		VkPhysicalDeviceProperties physicalDeviceProperties;				// physicalDevice 선택시 메모리 속성과 함께 얻어준다.
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 lightSpace;
		alignas(16) glm::vec4 lightPos;
	};

	struct ShadowUniformBufferObject {
		alignas(16) glm::mat4 depthMVP;
	};

	struct OffscreenPass {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		VkImage shadowImage;
		VkDeviceMemory shadowMemory;
		VkImageView shadowImageView;
		VkRenderPass renderPass;
		VkSampler depthSampler;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
	};

	struct PushConstantData {
		alignas(16) glm::mat4 model;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord && color == other.color;
		}
	};

	struct SkinVertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 color;
		glm::vec4 jointIndices;
		glm::vec4 jointWeights;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions();
	};

	struct ShadowMapVertex {
		glm::vec3 pos;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();
	};

	class Shader
	{
	private:
		vkf::Device* fDevice = nullptr;

		VkShaderModule vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule fragShaderModule = VK_NULL_HANDLE;

	public:
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	public:
		Shader(vkf::Device& fDevice, std::string vertFilename, std::string fragFilename);
		~Shader();

	private:
		void createShader(const std::string& vertFilename, const std::string& fragFilename);
		void destroy();
		VkShaderModule createShaderModule(const std::vector<char>& code);
	};

	class MeshBuffer
	{
	private:
		vkf::Device* fDevice = nullptr;

	public:
		uint32_t indexCount = 0;

		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

	public:
		void loadFromBuffer(vkf::Device& fDevice, const std::vector<vkf::Vertex>& vertices, const std::vector<uint32_t>& indices);
		void loadFromBuffer(vkf::Device& fDevice, const std::vector<vkf::SkinVertex>& vertices, const std::vector<uint32_t>& indices);
		void loadFromObjFile(vkf::Device& fDevice, std::string filename);
		void destroy();

	private:
		std::pair<std::vector<vkf::Vertex>, std::vector<uint32_t>> loadObjModel(std::string filename);
		void createVertexBuffer(const std::vector<vkf::Vertex>& vertices);
		void createVertexBuffer(const std::vector<vkf::SkinVertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
	};

	class BufferObject
	{
	private:
		vkf::Device* fDevice = nullptr;

		std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> buffers{};
		std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> buffersMemory{};
		std::array<void*, MAX_FRAMES_IN_FLIGHT> buffersMapped{};

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	public:
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets{};

	public:
		void createUniformBufferObjects(vkf::Device& fDevice, VkDescriptorSetLayout descriptorSetLayout);		// ubo, ssbo 중 하나만 생성 및 호출할 것
		void createShaderStorageBufferObjects(vkf::Device& fDevice, VkDeviceSize bufferSize, VkDescriptorSetLayout descriptorSetLayout);
		void createShadowUniformBufferObjects(vkf::Device& fDevice, VkDescriptorSetLayout descriptorSetLayout);
		void destroy();

		void copyTo(const void* data, VkDeviceSize size, uint32_t currentFrame);
		void updateUniformBuffer(const UniformBufferObject& ubo, uint32_t currentFrame);
		void updateShadowUniformBuffer(const ShadowUniformBufferObject& ubo, uint32_t currentFrame);
		VkDescriptorPool getDescriptorPool();
		VkDescriptorSet getDescriptorSet(uint32_t currentFrame);

	private:
		void createBuffers(VkDeviceSize bufferSize, VkBufferUsageFlags usage);
		void createDescriptorPool(VkDescriptorType type);
		void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDeviceSize bufferSize, VkDescriptorType descriptorType);
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
		void loadFromFile(vkf::Device& fDevice, std::string filename, VkDescriptorPool samplerDescriptorPool, VkDescriptorSetLayout samplerDescriptorSetLayout);
		void loadFromBuffer(vkf::Device& fDevice, void* buffer, VkDeviceSize bufferSize, uint32_t texWidth, uint32_t texHeight, VkDescriptorPool samplerDescriptorPool, VkDescriptorSetLayout samplerDescriptorSetLayout);
		void destroy();

	private:
		void createTextureImage(std::string filename);
		void createTextureImage(void* buffer, VkDeviceSize bufferSize, uint32_t texWidth, uint32_t texHeight);
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
}