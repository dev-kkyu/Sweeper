#pragma once

#include <memory>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tinygltf
{
	class Node;
	class Model;
}

class GLTFCollisionModel
{
public:
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
	};

	// A primitive contains the data for a single draw call
	// primitive�� single draw call�� ���� �����Ͱ� ���ԵǾ� �ִ�.
	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
	};

	// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
	// ������ geometry�� ���������� ���Ե� �� ������, ������ ������ primitive�� ������ �� �ִ�.
	struct Mesh {
		std::vector<Primitive> primitives;
	};

	// A node represents an object in the glTF scene graph
	// ���� glTF �� �׷����� ��ü�� ��Ÿ����.
	struct Node {
		std::shared_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;
		Mesh mesh;
		glm::mat4 matrix;
	};

private:
public:
	std::vector<Vertex> vertexBuffer;
	std::vector<uint32_t> indexBuffer;
	std::vector<std::shared_ptr<Node>> nodes;

public:
	GLTFCollisionModel(std::string filename);

private:
	void loadglTFFile(std::string filename);
	void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, const std::shared_ptr<GLTFCollisionModel::Node>& parent);

};

