#pragma once
#include"AppComponents/IService.h"
#include"ScriptableBase.h"
class ScriptService : public AppComponents::IService , public ScriptableBase
{
public:
	virtual void Tick(double deltatime);
	virtual void Start();
	virtual void End();

	virtual void Init();

	inline void SetServiceName(std::string n) { Name = std::move(n); }
	inline void SetTickInterval(double d) { TickInterval = d; }
	inline double GetTickInterval()const { return TickInterval; }
};

