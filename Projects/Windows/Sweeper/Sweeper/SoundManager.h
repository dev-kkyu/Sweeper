#pragma once

#include <array>

namespace FMOD
{
	class System;
	class Channel;
	class Sound;
}

class SoundManager
{
private:
	FMOD::System* ssystem;

	FMOD::Channel* backgroundChannel;

	std::array<FMOD::Sound*, 11> sounds;

private:
	// ΩÃ±€≈Ê
	SoundManager();
	~SoundManager();

	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

private:
	void loadSoundFiles();

public:
	static SoundManager& getInstance();

public:
	void update();

	void playBGM();
	void stopBGM();

	void playNormalAttackSound();
	void playWarriorSkillSound();
	void playArcherAttackSound();
	void playArcherSkillSound();
	void playMageAttackSound();
	void playMageSkillSound();
	void playHealerSkillSound();

	void playDashSound();
	void playHitSound();

	void playButtonSound();

};

