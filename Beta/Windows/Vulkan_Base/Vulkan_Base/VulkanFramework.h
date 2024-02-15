#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

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

	VkImageView createImageView(Device& fDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImage(Device& fDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(Device& fDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(Device& fDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void generateMipmaps(Device& fDevice, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void createBuffer(Device& fDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands(Device& fDevice);
	void endSingleTimeCommands(Device& fDevice, VkCommandBuffer commandBuffer);

	void copyBuffer(Device& fDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(Device& fDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(Device& fDevice, const std::vector<char>& code);
}


