#pragma once
#include"Macros.h"
#include<vector>
#include<functional>
#include<mutex>
#include<atomic>
#include"InputEnums.h"
#include"KeyEnquirer.h"
#include"Structures/Delegate.h"
namespace AppComponents {
namespace InputManagement
{
	/*
	* base class for input types
	* all input classes are registered here for central control
	* you shouold call this class to dispatch all input bindings
	*/
	class  ARCINPUT_API Input
	{
		using Dispatcher_t = void (*)();
		static std::vector<Dispatcher_t> InputDispatchers;
		static std::mutex Input_mtx;
	protected:
		static void Register(Dispatcher_t dispatcher);
	public:
		//dispatchs registered inputs
		static void DispatchInputs();
	};
	
	//represents a event that can be called when a key state is changed[ action based]
	template<EKeys key>
	class  ActionInput : protected Input
	{
	public:
		ActionInput()
		{
			RegisterInput();
		}
		static void RegisterInput()
		{
			bool expected{false};
			if (bRegistered.compare_exchange_strong(expected, true))
			{
				Input::Register(DispatchEvents);
			}
		}
		static void BindEvent(EKeyAcions action ,size_t id, std::function<void()>&& e)
		{
			std::lock_guard Lock{ Action_mtx };
			switch (action)
			{
			case EKeyAcions::PRESSED:
				OnPressedEvents.Bind(id,std::move(e));
				break;
			case EKeyAcions::RELEASED:
				OnReleasedEvents.Bind(id,std::move(e));
				break;
			}
		}
		static EKeyAcions GetCurrentKeyState()noexcept 
		{
			return CurrentKeyState.load(std::memory_order_acquire);
		}
		
	private:
		static void DispatchEvents()
		{
			EKeyAcions state{};
			switch (KeyEnquirer::GetKeyState(key))
			{
			case EKeyAcions::PRESSED:
				state = EKeyAcions::RELEASED;
				if (CurrentKeyState.compare_exchange_strong(state, EKeyAcions::PRESSED,std::memory_order_acq_rel,std::memory_order_relaxed))
				{
					std::lock_guard Lock{ Action_mtx };
					OnPressedEvents.Call();
				}
				break;
			case EKeyAcions::RELEASED:
				state = EKeyAcions::PRESSED;
				if (CurrentKeyState.compare_exchange_strong(state, EKeyAcions::RELEASED, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					std::lock_guard Lock{ Action_mtx };
					OnReleasedEvents.Call();
				}
				break;
			}
		}
		
		inline static Structures::TMultiDelegate<> OnPressedEvents{};
		inline static Structures::TMultiDelegate<> OnReleasedEvents{};
		inline static std::atomic<EKeyAcions> CurrentKeyState{ EKeyAcions::RELEASED };
		inline static std::atomic<bool> bRegistered{ false };
		inline static std::mutex Action_mtx{};//protects the vectors
	};

	template<typename Axis_t, std::enable_if_t<std::is_same_v<Axis_t, EAxis> || std::is_same_v<Axis_t, EKeys>, Axis_t> axis>
	class  FAxis 
	{
		
		std::atomic<float> Value{};
	public:
		using Axis_Type = typename Axis_t;
		const Axis_Type AxisName{ axis };

		void Update() {};
		void SetValue(float val) { Value.store(val, std::memory_order_release); };
		float GetValue()const noexcept { return Value.load(std::memory_order_acquire); };
	};
	
	template<>
	inline void FAxis<EAxis,EAxis::MOUSE_X>::Update()
	{
		SetValue(static_cast<float>(KeyEnquirer::GetMousePos_X()));
	}
	template<>
	inline void FAxis<EAxis, EAxis::MOUSE_Y>::Update()
	{
		SetValue(static_cast<float>(KeyEnquirer::GetMousePos_Y()));
	}

	template<EKeys AxisKey>
	class  FAxis<EKeys, AxisKey>
	{
		float Value{};
	public:
		using Axis_Type = typename EKeys;
		const Axis_Type AxisName{ AxisKey };

		void Update() {
			SetValue(static_cast<float>(KeyEnquirer::GetKeyState(AxisKey))? 1.f:0.f);
		};
		void SetValue(float val) { Value = val; };
		float GetValue()const noexcept { return Value; };
	};

	//represents a event that can be called continously 
	template<typename Axis_t, std::enable_if_t<std::is_same_v<Axis_t, EAxis> || std::is_same_v<Axis_t, EKeys>, Axis_t> axis>
	class  AxisInput: protected Input
	{
	public:
		AxisInput()
		{
			Input::Register(DispatchEvents);
		}
	
		static void RegisterInput()
		{
			bool expected{false};
			if (bRegistered.compare_exchange_strong(expected, true))
			{
				Input::Register(DispatchEvents);
			}
		}
		static void BindEvent(size_t id, std::function<void(float)>&& e)
		{
			std::lock_guard Lock{Axis_mtx};
			Events.Bind(id, std::move(e));
		}
		static float GetCurrentAxisValue()noexcept
		{
			return _Axis.GetValue();
		}
		static void SetAxisValue(float newValue)
		{
			_Axis.SetValue(newValue);
		}
	private:
		static void DispatchEvents()
		{
			std::lock_guard Lock{ Axis_mtx };
			_Axis.Update();
			Events.Call(_Axis.GetValue());
		}
		inline static Structures::TMultiDelegate<float> Events{};
		inline static std::mutex Axis_mtx;
		inline static FAxis<Axis_t,axis> _Axis{};
		inline static std::atomic<bool> bRegistered{ false };
	};
	
}
}
	
