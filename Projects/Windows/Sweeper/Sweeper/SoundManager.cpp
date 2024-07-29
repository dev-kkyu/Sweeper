#include "SoundManager.h"

#ifdef _WIN64

#include <FMOD/fmod.hpp>

#include <string>

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
	std::string filename[13]{ "bgm", "normalattack", "warriorskill", "archerattack", "archerskill",
		"mageattack", "mageskill", "healerskill", "dash", "playerhit", "monsterhit", "bossskill", "button" };
	for (int i = 0; i < sounds.size(); ++i) {
		std::string name = "./sounds/" + filename[i] + ".mp3";
		if (i < 1)
			ssystem->createSound(name.c_str(), FMOD_LOOP_NORMAL, nullptr, &sounds[i]);
		else
			ssystem->createSound(name.c_str(), FMOD_DEFAULT, nullptr, &sounds[i]);
	}
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
	stopBGM();
	ssystem->playSound(sounds[0], nullptr, false, &backgroundChannel);
}

void SoundManager::stopBGM()
{
	if (backgroundChannel)
		backgroundChannel->stop();
}

void SoundManager::playNormalAttackSound()
{
	ssystem->playSound(sounds[1], nullptr, false, nullptr);
}

void SoundManager::playWarriorSkillSound()
{
	FMOD::Channel* channel;
	ssystem->playSound(sounds[2], nullptr, false, &channel);
	channel->setVolume(0.5f);
}

void SoundManager::playArcherAttackSound()
{
	ssystem->playSound(sounds[3], nullptr, false, nullptr);
}

void SoundManager::playArcherSkillSound()
{
	ssystem->playSound(sounds[4], nullptr, false, nullptr);
}

void SoundManager::playMageAttackSound()
{
	ssystem->playSound(sounds[5], nullptr, false, nullptr);
}

void SoundManager::playMageSkillSound()
{
	ssystem->playSound(sounds[6], nullptr, false, nullptr);
}

void SoundManager::playHealerSkillSound()
{
	FMOD::Channel* channel;
	ssystem->playSound(sounds[7], nullptr, false, &channel);
	channel->setVolume(0.75f);
}

void SoundManager::playDashSound()
{
	ssystem->playSound(sounds[8], nullptr, false, nullptr);
}

void SoundManager::playPlayerHitSound()
{
	ssystem->playSound(sounds[9], nullptr, false, nullptr);
}

void SoundManager::playMonsterHitSound()
{
	FMOD::Channel* channel;
	ssystem->playSound(sounds[10], nullptr, false, &channel);
	channel->setVolume(0.75f);
}

void SoundManager::playBossSkillSound()
{
	FMOD::Channel* channel;
	ssystem->playSound(sounds[11], nullptr, false, &channel);
	channel->setVolume(0.5f);
}

void SoundManager::playButtonSound()
{
	ssystem->playSound(sounds[12], nullptr, false, nullptr);
}

#else

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
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
}

void SoundManager::playBGM()
{
}

void SoundManager::stopBGM()
{
}

void SoundManager::playNormalAttackSound()
{
}

void SoundManager::playWarriorSkillSound()
{
}

void SoundManager::playArcherAttackSound()
{
}

void SoundManager::playArcherSkillSound()
{
}

void SoundManager::playMageAttackSound()
{
}

void SoundManager::playMageSkillSound()
{
}

void SoundManager::playHealerSkillSound()
{
}

void SoundManager::playDashSound()
{
}

void SoundManager::playHitSound()
{
}

void SoundManager::playButtonSound()
{
}

#endif // _WIN64
