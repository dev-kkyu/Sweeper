#include "GameObject.h"

GameObject::GameObject(vkf::Device& fDevice)
	: fDevice{ fDevice }
{
	initialize();
}

GameObject::~GameObject()
{
	release();
}

void GameObject::initialize()
{
}

void GameObject::update(float elapsedTime, uint32_t currentFrame)
{

}

void GameObject::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void GameObject::release()
{
	vkDestroyBuffer(fDevice.device, indexBuffer, nullptr);
	vkFreeMemory(fDevice.device, indexBufferMemory, nullptr);

	vkDestroyBuffer(fDevice.device, vertexBuffer, nullptr);
	vkFreeMemory(fDevice.device, vertexBufferMemory, nullptr);
}

void GameObject::setBuffer(std::vector<vkf::Vertex> vertices, std::vector<uint32_t> indices)
{
	vertexCount = static_cast<uint32_t>(vertices.size());
	indexCount = static_cast<uint32_t>(indices.size());

	createVertexBuffer(vertices);
	createIndexBuffer(indices);
}

void GameObject::createVertexBuffer(std::vector<vkf::Vertex> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(fDevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(fDevice.device, stagingBufferMemory);

	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	vkf::copyBuffer(fDevice, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(fDevice.device, stagingBuffer, nullptr);
	vkFreeMemory(fDevice.device, stagingBufferMemory, nullptr);
}

void GameObject::createIndexBuffer(std::vector<uint32_t> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(fDevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(fDevice.device, stagingBufferMemory);

	vkf::createBuffer(fDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	vkf::copyBuffer(fDevice, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(fDevice.device, stagingBuffer, nullptr);
	vkFreeMemory(fDevice.device, stagingBufferMemory, nullptr);
}
