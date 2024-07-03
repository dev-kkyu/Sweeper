#pragma once

#include "GLTFCollisionModel.h"

// 싱글톤 클래스
class Map
{
private:
	const GLTFCollisionModel model;

	std::vector<BoundingBox> boundingBox;

private:			// 싱글톤으로 만들기 위해 생성자 외부 노출 X
	Map();
	~Map();

	// 복사는 할 일 없으니 막아둔다.	// 싱글톤을 위해서도 막아준다.
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

public:
	static Map& getInstance();	// 싱글톤 객체 얻기 위한 static 함수

	const std::vector<BoundingBox>& getBoundingBox() const;

private:
	void copyBoundingBoxByNode(const std::shared_ptr<GLTFCollisionModel::Node> node);

};

