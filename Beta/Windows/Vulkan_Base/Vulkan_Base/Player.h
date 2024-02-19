#pragma once

#include "GameObject.h"

class Player : public GameObject
{
protected:

public:
	Player(vkf::Device& fDevice);
	~Player();
};

