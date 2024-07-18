#pragma once

#include "GameObjectBase.h"

#include "protocol.h"	// BOSS_STATE

class BossObject;
class BossState
{
protected:
	BOSS_STATE state;
	BossObject& boss;

public:
	BossState(BossObject& boss);
	virtual ~BossState() = default;

	virtual void enter() = 0;
	virtual void update(float elapsedTime) = 0;
	virtual void exit() = 0;
	virtual BOSS_STATE getState() const final;
};

class BossSLEEP : public BossState
{
public:
	BossSLEEP(BossObject& boss);
	virtual ~BossSLEEP() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossWAKEUP : public BossState
{
public:
	BossWAKEUP(BossObject& boss);
	virtual ~BossWAKEUP() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossIDLE : public BossState
{
public:
	BossIDLE(BossObject& boss);
	virtual ~BossIDLE() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossMOVE : public BossState
{
public:
	BossMOVE(BossObject& boss);
	virtual ~BossMOVE() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossLEFTPUNCH : public BossState
{
public:
	BossLEFTPUNCH(BossObject& boss);
	virtual ~BossLEFTPUNCH() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossRIGHTPUNCH : public BossState
{
public:
	BossRIGHTPUNCH(BossObject& boss);
	virtual ~BossRIGHTPUNCH() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossPUNCHDOWN : public BossState
{
public:
	BossPUNCHDOWN(BossObject& boss);
	virtual ~BossPUNCHDOWN() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossDIE : public BossState
{
public:
	BossDIE(BossObject& boss);
	virtual ~BossDIE() = default;

	virtual void enter() override;
	virtual void update(float elapsedTime) override;
	virtual void exit() override;
};

class BossObject : public GameObjectBase
{
	friend class BossState;

private:
	// 상태 관리
	std::unique_ptr<BossState> currentState;
	std::unique_ptr<BossState> nextState;

public:
	BossObject(Room* parentRoom, int m_id);
	virtual ~BossObject();

	virtual void initialize() override;
	virtual bool update(float elapsedTime) override;
	virtual void release() override;
	virtual void onHit(const GameObjectBase& other, int damage) override;

private:
	virtual void changeSLEEPState() final;
	virtual void changeWAKEUPState() final;
	virtual void changeIDLEState() final;
	virtual void changeMOVEState() final;
	virtual void changeLEFTPUNCHState() final;
	virtual void changeRIGHTPUNCHState() final;
	virtual void changePUNCHDOWNState() final;
	virtual void changeDIEState() final;

};

