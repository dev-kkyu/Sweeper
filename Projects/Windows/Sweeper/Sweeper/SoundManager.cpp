#include "SoundManager.h"

#include <FMOD/fmod.hpp>

SoundManager::SoundManager()
{
	// 사운드 시스템 생성
	FMOD::System_Create(&ssystem);
	ssystem->init(32, FMOD_INIT_NORMAL, nullptr);

	backgroundChannel = nullptr;

	loadSoundFiles();
}

SoundManager::~SoundManager()
{
	for (int i = 0; i < sounds.size(); ++i)
		sounds[i]->release();
	ssystem->close();
	ssystem->release();
}

void SoundManager::loadSoundFiles()
{
}

SoundManager& SoundManager::getInstance()
{
	static SoundManager instance;
	return instance;
}

void SoundManager::update()
{
	ssystem->update();
}

void SoundManager::playBGM()
{
}
