#include "GLTFSkinModelObject.h"

#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

using Node = VulkanGLTFSkinModel::Node;
using AnimationSampler = VulkanGLTFSkinModel::AnimationSampler;
using AnimationChannel = VulkanGLTFSkinModel::AnimationChannel;
using Primitive = VulkanGLTFSkinModel::Primitive;
using TextureID = VulkanGLTFSkinModel::TextureID;

GLTFSkinModelObject::GLTFSkinModelObject()
{
}

GLTFSkinModelObject::~GLTFSkinModelObject()
{
	release();
}

void GLTFSkinModelObject::initialize()
{
}

void GLTFSkinModelObject::update(float elapsedTime, uint32_t currentFrame)
{
	updateAnimation(elapsedTime * animateSpeed, currentFrame);
}

void GLTFSkinModelObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	model->bindBuffers(commandBuffer);

	// Render all nodes at top-level
	for (auto& node : nodes)
	{
		drawNode(commandBuffer, pipelineLayout, currentFrame, node, modelTransform);
	}
}

void GLTFSkinModelObject::release()
{
	for (auto& skin : skins) {
		skin.ssbo.destroy();
	}
}

void GLTFSkinModelObject::initModel(VulkanGLTFSkinModel& model, VkDescriptorSetLayout ssboDescriptorSetLayout)
{
	this->model = &model;

	const tinygltf::Model& glTFInput = *(this->model->glTFInput);
	const tinygltf::Scene& scene = glTFInput.scenes[0];
	uint32_t indexBufferCount = 0;
	for (size_t i = 0; i < scene.nodes.size(); i++)
	{
		const tinygltf::Node& node = glTFInput.nodes[scene.nodes[i]];
		loadNode(node, nullptr, scene.nodes[i], indexBufferCount);
	}
	loadSkins(ssboDescriptorSetLayout);
	loadAnimations();
	// Calculate initial pose
	for (uint32_t k = 0; k < MAX_FRAMES_IN_FLIGHT; ++k) {
		for (auto& node : nodes) {
			updateJoints(node, k);
		}
	}
}

void GLTFSkinModelObject::setAnimateSpeed(float speed)
{
	animateSpeed = speed;
}

void GLTFSkinModelObject::changeAnimationClip()
{
	activeAnimation = (activeAnimation + 1) % animations.size();
	animations[activeAnimation].currentTime = 0.f;
}

void GLTFSkinModelObject::setAnimationClip(uint32_t animationIndex)
{
	activeAnimation = animationIndex;
	animations[activeAnimation].currentTime = 0.f;
}

BoundingBox GLTFSkinModelObject::getBoundingBox() const
{
	const glm::vec3 myPos = getPosition();
	BoundingBox boundingBox;
	boundingBox.setBound(1.f, 0.1f, myPos.z + collisionRadius, myPos.z - collisionRadius, myPos.x - collisionRadius, myPos.x + collisionRadius);
	return boundingBox;
}

void GLTFSkinModelObject::drawBoundingBox(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	getBoundingBox().draw(commandBuffer, pipelineLayout);
}

std::shared_ptr<Node> GLTFSkinModelObject::findNode(const std::shared_ptr<Node>& parent, uint32_t index) const
{
	std::shared_ptr<Node> nodeFound = nullptr;
	if (parent->index == index)
	{
		return parent;
	}
	for (auto& child : parent->children)
	{
		nodeFound = findNode(child, index);
		if (nodeFound)
		{
			break;
		}
	}
	return nodeFound;
}

std::shared_ptr<Node> GLTFSkinModelObject::nodeFromIndex(uint32_t index) const
{
	std::shared_ptr<Node> nodeFound = nullptr;
	for (auto& node : nodes)
	{
		nodeFound = findNode(node, index);
		if (nodeFound)
		{
			break;
		}
	}
	return nodeFound;
}

void GLTFSkinModelObject::loadSkins(VkDescriptorSetLayout ssboDescriptorSetLayout)
{
	tinygltf::Model& input = *(model->glTFInput);

	skins.resize(input.skins.size());

	for (size_t i = 0; i < input.skins.size(); i++)
	{
		const tinygltf::Skin& glTFSkin = input.skins[i];

		skins[i].name = glTFSkin.name;
		// Find the root node of the skeleton
		skins[i].skeletonRoot = nodeFromIndex(glTFSkin.skeleton);

		// Find joint nodes
		for (int jointIndex : glTFSkin.joints)
		{
			std::shared_ptr<Node> node = nodeFromIndex(jointIndex);
			if (node)
			{
				skins[i].joints.push_back(node);
			}
		}

		// Get the inverse bind matrices from the buffer associated to this skin
		if (glTFSkin.inverseBindMatrices > -1)
		{
			const tinygltf::Accessor& accessor = input.accessors[glTFSkin.inverseBindMatrices];
			const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
			skins[i].inverseBindMatrices.resize(accessor.count);
			memcpy(skins[i].inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));

			// 이 스킨에 대한 역 바인드 행렬을 담을 공간을 Shader Storage Buffer Object에 생성 및 저장
			skins[i].ssbo.createShaderStorageBufferObjects(*model->fDevice, sizeof(glm::mat4) * skins[i].inverseBindMatrices.size(), ssboDescriptorSetLayout);
			for (uint32_t k = 0; k < MAX_FRAMES_IN_FLIGHT; ++k) {
				skins[i].ssbo.copyTo(skins[i].inverseBindMatrices.data(), sizeof(glm::mat4) * skins[i].inverseBindMatrices.size(), k);
			}
		}
	}
}

void GLTFSkinModelObject::loadAnimations()
{
	tinygltf::Model& input = *(model->glTFInput);

	animations.resize(input.animations.size());

	for (size_t i = 0; i < input.animations.size(); i++)
	{
		const tinygltf::Animation& glTFAnimation = input.animations[i];
		animations[i].name = glTFAnimation.name;

		// Samplers
		animations[i].samplers.resize(glTFAnimation.samplers.size());
		for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
		{
			const tinygltf::AnimationSampler& glTFSampler = glTFAnimation.samplers[j];
			AnimationSampler& dstSampler = animations[i].samplers[j];
			dstSampler.interpolation = glTFSampler.interpolation;

			// Read sampler keyframe input time values
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.input];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
				{
					dstSampler.inputs.push_back(buf[index]);
				}
				// Adjust animation's start and end times
				for (auto input : animations[i].samplers[j].inputs)
				{
					if (input < animations[i].start)
					{
						animations[i].start = input;
					};
					if (input > animations[i].end)
					{
						animations[i].end = input;
					}
				}
			}

			// Read sampler keyframe output translate/rotate/scale values
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.output];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3: {
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4: {
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(buf[index]);
					}
					break;
				}
				default: {
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}
		}

		// Channels
		animations[i].channels.resize(glTFAnimation.channels.size());
		for (size_t j = 0; j < glTFAnimation.channels.size(); j++)
		{
			const tinygltf::AnimationChannel& glTFChannel = glTFAnimation.channels[j];
			AnimationChannel& dstChannel = animations[i].channels[j];
			dstChannel.path = glTFChannel.target_path;
			dstChannel.samplerIndex = glTFChannel.sampler;
			dstChannel.node = nodeFromIndex(glTFChannel.target_node);
		}
	}
}

void GLTFSkinModelObject::loadNode(const tinygltf::Node& inputNode, const std::shared_ptr<Node>& parent, uint32_t nodeIndex, uint32_t& indexBufferCount)
{
	const tinygltf::Model& input = *(model->glTFInput);

	std::shared_ptr<Node> node = std::make_shared<Node>();
	node->parent = parent;
	node->matrix = glm::mat4(1.0f);
	node->index = nodeIndex;
	node->skin = inputNode.skin;

	// Get the local node matrix
	// It's either made up from translation, rotation, scale or a 4x4 matrix
	if (inputNode.translation.size() == 3)
	{
		node->translation = glm::make_vec3(inputNode.translation.data());
	}
	if (inputNode.rotation.size() == 4)
	{
		glm::quat q = glm::make_quat(inputNode.rotation.data());
		node->rotation = glm::mat4(q);
	}
	if (inputNode.scale.size() == 3)
	{
		node->scale = glm::make_vec3(inputNode.scale.data());
	}
	if (inputNode.matrix.size() == 16)
	{
		node->matrix = glm::make_mat4x4(inputNode.matrix.data());
	};

	// Load node's children
	if (inputNode.children.size() > 0)
	{
		for (size_t i = 0; i < inputNode.children.size(); i++)
		{
			loadNode(input.nodes[inputNode.children[i]], node, inputNode.children[i], indexBufferCount);
		}
	}

	// 모델에서는 vertex, index buffer 만들어 줬고, 노드마다의 index와 offset을 구해준다. -> Primitive
	if (inputNode.mesh > -1)
	{
		const tinygltf::Mesh& mesh = input.meshes[inputNode.mesh];
		// Iterate through all primitives of this node's mesh
		for (size_t i = 0; i < mesh.primitives.size(); i++)
		{
			const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
			uint32_t                   firstIndex = indexBufferCount;
			uint32_t                   indexCount = 0;

			{
				const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];

				indexCount += static_cast<uint32_t>(accessor.count);
			}
			Primitive primitive{};
			primitive.firstIndex = firstIndex;
			primitive.indexCount = indexCount;
			primitive.materialIndex = glTFPrimitive.material;
			node->mesh.primitives.push_back(primitive);

			indexBufferCount += indexCount;
		}
	}

	if (parent)
	{
		parent->children.push_back(node);
	}
	else
	{
		nodes.push_back(node);
	}
}

glm::mat4 GLTFSkinModelObject::getNodeMatrix(const std::shared_ptr<Node>& node) const
{
	glm::mat4				nodeMatrix = node->getLocalMatrix();
	std::shared_ptr<Node>	currentParent = node->parent;
	while (currentParent)
	{
		nodeMatrix = currentParent->getLocalMatrix() * nodeMatrix;
		currentParent = currentParent->parent;
	}
	return nodeMatrix;
}

// 현재 애니메이션 프레임에서 joint 행렬을 업데이트하고 GPU에 전달
void GLTFSkinModelObject::updateJoints(const std::shared_ptr<Node>& node, uint32_t currentFrame)
{
	if (node->skin > -1)
	{
		// Update the joint matrices
		glm::mat4 inverseTransform = glm::inverse(getNodeMatrix(node));
		Skin& skin = skins[node->skin];
		size_t numJoints = (uint32_t)skin.joints.size();
		std::vector<glm::mat4> jointMatrices(numJoints);
		for (size_t i = 0; i < numJoints; i++)
		{
			jointMatrices[i] = getNodeMatrix(skin.joints[i]) * skin.inverseBindMatrices[i];
			jointMatrices[i] = inverseTransform * jointMatrices[i];
		}
		// Update ssbo
		skin.ssbo.copyTo(jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4), currentFrame);
	}

	for (auto& child : node->children)
	{
		updateJoints(child, currentFrame);
	}
}

void GLTFSkinModelObject::updateAnimation(float elapsedTime, uint32_t currentFrame)
{
	if (activeAnimation > static_cast<uint32_t>(animations.size()) - 1)
	{
		std::cout << "No animation with index " << activeAnimation << std::endl;
		return;
	}
	Animation& animation = animations[activeAnimation];
	animation.currentTime += elapsedTime;
	if (animation.currentTime > animation.end)
	{
		animation.currentTime -= animation.end;
	}

	for (auto& channel : animation.channels)
	{
		const AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if (sampler.interpolation != "LINEAR")
			{
				std::cout << "This sample only supports linear interpolations\n";
				continue;
			}

			// Get the input keyframe values for the current time stamp
			if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1]))
			{
				float a = (animation.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (channel.path == "translation")
				{
					channel.node->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
				if (channel.path == "rotation")
				{
					glm::quat q1;
					q1.x = sampler.outputsVec4[i].x;
					q1.y = sampler.outputsVec4[i].y;
					q1.z = sampler.outputsVec4[i].z;
					q1.w = sampler.outputsVec4[i].w;

					glm::quat q2;
					q2.x = sampler.outputsVec4[i + 1].x;
					q2.y = sampler.outputsVec4[i + 1].y;
					q2.z = sampler.outputsVec4[i + 1].z;
					q2.w = sampler.outputsVec4[i + 1].w;

					channel.node->rotation = glm::normalize(glm::slerp(q1, q2, a));
				}
				if (channel.path == "scale")
				{
					channel.node->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
			}
		}
	}
	for (auto& node : nodes)
	{
		updateJoints(node, currentFrame);
	}
}

void GLTFSkinModelObject::drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame, const std::shared_ptr<Node>& node, const glm::mat4& worldMatrix)
{
	if (node->mesh.primitives.size() > 0)
	{
		// Pass the node's matrix via push constants
		// Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
		glm::mat4				nodeMatrix = node->matrix;
		std::shared_ptr<Node>	currentParent = node->parent;
		while (currentParent)
		{
			nodeMatrix = currentParent->matrix * nodeMatrix;
			currentParent = currentParent->parent;
		}
		// 월드 좌표계로 이동
		nodeMatrix = worldMatrix * nodeMatrix;
		// Pass the final matrix to the vertex shader using push constants
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
		// 현재 노드의 skin data에 해당하는 ssbo를 3번에 바인드 한다.
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &skins[node->skin].ssbo.descriptorSets[currentFrame], 0, nullptr);
		for (Primitive& primitive : node->mesh.primitives)
		{
			if (primitive.indexCount > 0)
			{
				// Get the texture index for this primitive
				TextureID texture = model->textures[model->materials[primitive.materialIndex].baseColorTextureIndex];
				// 현재 프리미티브 텍스처의 디스크립터 셋을 1번에 바인드 한다.
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &model->images[texture.imageIndex].texture.samplerDescriptorSet, 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
			}
		}
	}
	for (auto& child : node->children)
	{
		drawNode(commandBuffer, pipelineLayout, currentFrame, child, worldMatrix);
	}
}
