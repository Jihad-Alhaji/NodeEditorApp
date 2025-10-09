#pragma once
#include<string>
namespace AppComponents
{
	class IService
	{
	protected:
		std::string Name;
		bool bCanEverTick : 1 = true;
		bool bAllowTick : 1 = true;
		double TickInterval = 0.05;

		virtual void Tick(double deltatime) {};
		virtual void Start() {};
		virtual void End() {};

		//called by the provider to do initialization, called onec and is not related to activation
		virtual void Init() {};

		
	private:
		//service has started
		bool bActive{ false };

		double ElapsedTime{};//the time elapseped since we last ticked, used when custom tick rate is set
		void Tick_Internal(double deltatime);

		friend class IServiceProvider;

	public:
		IService() = default;
		void Deactivate();
		void Activate();

		//has the service started
		inline bool IsActive()const { return bActive; }
		
		inline bool CanTick()const { return bActive && bCanEverTick && bAllowTick; }

		inline void SetTickEnabled(bool  newTick) { bAllowTick = newTick; }
		inline bool GetTickEnabled()const { return bAllowTick; }
		inline std::string_view GetServiceName()const { return Name; }
	};

}