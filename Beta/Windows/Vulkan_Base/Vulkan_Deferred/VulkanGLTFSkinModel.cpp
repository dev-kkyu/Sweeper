#include "VulkanGLTFSkinModel.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

glm::mat4 VulkanGLTFSkinModel::Node::getLocalMatrix()
{
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

void VulkanGLTFSkinModel::destroy()
{
	buffer.destroy();

	for (Image image : images) {
		image.texture.destroy();
	}

	vkDestroyDescriptorPool(fDevice->logicalDevice, samplerDescriptorPool, nullptr);
}

void VulkanGLTFSkinModel::loadModel(vkf::Device& fDevice, VkDescriptorSetLayout samplerDescriptorSetLayout, std::string filename)
{
	this->fDevice = &fDevice;
	this->samplerDescriptorSetLayout = samplerDescriptorSetLayout;

	loadglTFFile(filename);
}

void VulkanGLTFSkinModel::bindBuffers(VkCommandBuffer commandBuffer)
{
	// All vertices and indices are stored in single buffers, so we only need to bind once
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanGLTFSkinModel::loadglTFFile(std::string filename)
{
	tinygltf::TinyGLTF	gltfContext;
	std::string			error, warning;

	glTFInput = std::make_shared<tinygltf::Model>();

	bool fileLoaded = false;

	// 확장자가 glb인지 검사하여 glb이면 binary로 열고, 아니면 .gltf로 간주
	size_t dotPosition = filename.find_last_of('.');
	if (dotPosition != std::string::npos && dotPosition != filename.length() - 1) {
		std::string extention = filename.substr(dotPosition + 1);
		if (extention == "glb") {
			fileLoaded = gltfContext.LoadBinaryFromFile(glTFInput.get(), &error, &warning, filename);
		}
		else {
			fileLoaded = gltfContext.LoadASCIIFromFile(glTFInput.get(), &error, &warning, filename);
		}
	}
	else {
		throw std::runtime_error("This file does not have extension.\n");
	}

	std::vector<vkf::SkinVertex>	vertexBuffer;
	std::vector<uint32_t>			indexBuffer;

	if (fileLoaded) {
		loadImages();
		loadMaterials();
		loadTextures();
		const tinygltf::Scene& scene = glTFInput->scenes[0];
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const tinygltf::Node node = glTFInput->nodes[scene.nodes[i]];
			loadMeshes(node, vertexBuffer, indexBuffer);
		}
	}
	else {
		throw std::runtime_error("Could not open the glTF file.\n\nMake sure the assets submodule has been checked out and is up-to-date.");
	}

	// 정점 및 인덱스 버퍼 생성 및 업로드
	// 전체 glTF 장면에 대해 하나의 단일 정점 버퍼와 하나의 단일 인덱스 버퍼 사용
	// (glTF 모델의) Primitives는 인덱스 오프셋을 사용하여 인덱스를 적용
	// Primitives 정보는 객체에서 각자 얻기로 한다. -> Node의 mesh가 들고있기때문

	buffer.loadFromBuffer(*fDevice, vertexBuffer, indexBuffer);
}

void VulkanGLTFSkinModel::createSamplerDescriptorPool(uint32_t setCount)
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = setCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = setCount;

	if (vkCreateDescriptorPool(fDevice->logicalDevice, &poolInfo, nullptr, &samplerDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanGLTFSkinModel::loadImages()
{
	// 이미지 개수만큼 DescriptorPool 미리 만들어 준다.
	createSamplerDescriptorPool(static_cast<uint32_t>(glTFInput->images.size()));
	// Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
	// loading them from disk, we fetch them from the glTF loader and upload the buffers
	images.resize(glTFInput->images.size());
	for (size_t i = 0; i < glTFInput->images.size(); i++) {
		tinygltf::Image& glTFImage = glTFInput->images[i];
		// Get the image data from the glTF loader
		unsigned char* buffer = nullptr;
		VkDeviceSize bufferSize = 0;
		bool deleteBuffer = false;
		// We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
		if (glTFImage.component == 3) {
			bufferSize = glTFImage.width * glTFImage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &glTFImage.image[0];
			for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
				memcpy(rgba, rgb, sizeof(unsigned char) * 3);
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else {
			buffer = &glTFImage.image[0];
			bufferSize = glTFImage.image.size();
		}
		// Load texture from image buffer
		images[i].texture.loadFromBuffer(*fDevice, buffer, bufferSize, glTFImage.width, glTFImage.width, samplerDescriptorPool, samplerDescriptorSetLayout);
		if (deleteBuffer) {
			delete[] buffer;
		}
	}
}

void VulkanGLTFSkinModel::loadTextures()
{
	textures.resize(glTFInput->textures.size());
	for (size_t i = 0; i < glTFInput->textures.size(); i++) {
		textures[i].imageIndex = glTFInput->textures[i].source;
	}
}

void VulkanGLTFSkinModel::loadMaterials()
{
	materials.resize(glTFInput->materials.size());
	for (size_t i = 0; i < glTFInput->materials.size(); i++) {
		// We only read the most basic properties required for our sample
		tinygltf::Material glTFMaterial = glTFInput->materials[i];
		// Get the base color factor
		if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
			materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
		}
		// Get base color texture index
		if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
			materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
		}
	}
}

void VulkanGLTFSkinModel::loadMeshes(const tinygltf::Node& inputNode, std::vector<vkf::SkinVertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
	// Load node's children
	if (inputNode.children.size() > 0)
	{
		for (size_t i = 0; i < inputNode.children.size(); i++)
		{
			loadMeshes(glTFInput->nodes[inputNode.children[i]], vertexBuffer, indexBuffer);
		}
	}

	// If the node contains mesh data, we load vertices and indices from the buffers
	// In glTF this is done via accessors and buffer views
	if (inputNode.mesh > -1)
	{
		const tinygltf::Mesh mesh = glTFInput->meshes[inputNode.mesh];
		// Iterate through all primitives of this node's mesh
		for (size_t i = 0; i < mesh.primitives.size(); i++)
		{
			const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
			uint32_t                   vertexStart = static_cast<uint32_t>(vertexBuffer.size());
			bool                       hasSkin = false;
			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				const uint16_t* jointIndicesBuffer = nullptr;
				const float* jointWeightsBuffer = nullptr;
				size_t          vertexCount = 0;

				// Get buffer data for vertex normals
				if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = glTFInput->bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(glTFInput->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}
				// Get buffer data for vertex normals
				if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = glTFInput->bufferViews[accessor.bufferView];
					normalsBuffer = reinterpret_cast<const float*>(&(glTFInput->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				// Get buffer data for vertex texture coordinates
				// glTF supports multiple sets, we only load the first one
				if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = glTFInput->bufferViews[accessor.bufferView];
					texCoordsBuffer = reinterpret_cast<const float*>(&(glTFInput->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// POI: Get buffer data required for vertex skinning
				// Get vertex joint indices
				if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& view = glTFInput->bufferViews[accessor.bufferView];
					jointIndicesBuffer = reinterpret_cast<const uint16_t*>(&(glTFInput->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				// Get vertex joint weights
				if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& view = glTFInput->bufferViews[accessor.bufferView];
					jointWeightsBuffer = reinterpret_cast<const float*>(&(glTFInput->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				hasSkin = (jointIndicesBuffer && jointWeightsBuffer);

				// Append data to model's vertex buffer
				for (size_t v = 0; v < vertexCount; v++)
				{
					vkf::SkinVertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
					vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
					vert.color = glm::vec3(1.0f);
					vert.jointIndices = hasSkin ? glm::vec4(glm::make_vec4(&jointIndicesBuffer[v * 4])) : glm::vec4(0.0f);
					vert.jointWeights = hasSkin ? glm::make_vec4(&jointWeightsBuffer[v * 4]) : glm::vec4(0.0f);
					vertexBuffer.push_back(vert);
				}
			}
			// Indices
			{
				const tinygltf::Accessor& accessor = glTFInput->accessors[glTFPrimitive.indices];
				const tinygltf::BufferView& bufferView = glTFInput->bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = glTFInput->buffers[bufferView.buffer];

				// glTF supports different component types of indices
				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}
		}
	}
}