#include "SceneBase.h"

SceneBase::SceneBase()
	: sceneType{ SCENE_TYPE::ERROR }, isEnd{ false }
{
}

SceneBase::~SceneBase()
{
}

SceneBase::SCENE_TYPE SceneBase::getSceneType() const
{
	return sceneType;
}
