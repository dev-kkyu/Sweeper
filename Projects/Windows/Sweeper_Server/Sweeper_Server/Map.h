#pragma once

#include "GLTFCollisionModel.h"

// �̱��� Ŭ����
class Map
{
private:
	const GLTFCollisionModel model;

	std::vector<BoundingBox> boundingBox;

private:			// �̱������� ����� ���� ������ �ܺ� ���� X
	Map();
	~Map();

	// ����� �� �� ������ ���Ƶд�.	// �̱����� ���ؼ��� �����ش�.
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

public:
	static Map& getInstance();	// �̱��� ��ü ��� ���� static �Լ�

	const std::vector<BoundingBox>& getBoundingBox() const;

private:
	void copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node);

};

