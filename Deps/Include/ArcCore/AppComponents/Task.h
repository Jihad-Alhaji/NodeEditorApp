#pragma once
#include"Structures/Delegate.h"
#include<string_view>
namespace AppComponents
{
	struct FTaskID
	{
		size_t ID = -1;
		FTaskID() = default;
		FTaskID(::std::string_view taskname)
		{
			SetID(taskname);
		}
		void SetID(::std::string_view taskname)
		{
			ID = ::std::hash<::std::string_view>().operator()(taskname);
		}

		bool operator==(const FTaskID& other)const
		{
			return ID == other.ID;
		}
		//this type can be easily used in place of a normal size_t 
		operator size_t()const { return ID; }
	};
	
	enum class ETaskStartResponse
	{
		SUCCESS,
		FAILED_RESOURCES,
		FAILED_REQUIREMENTS,
		FAILED
	};
	enum class ETaskEndResponse
	{
		COMPLETED,
		CANCELED
	};
	class Task
	{
		FTaskID TaskID;
		std::string TaskName;
		void EndTask_Internal(ETaskEndResponse response);
		bool IsActive : 1;
		bool IsCompleted : 1;

		void SetActive();
		void SetInActive();

		void Tick_Internal(double deltatime);
		double elapsedTime{};
	protected:
		//if true the main task action will test requirement before execution
		bool ShouldWatchRequirements : 1;

		bool CanEverTick : 1;
		bool AllowTick : 1;

		bool bAllowRestart : 1;

		//by the task manager
		bool bAutoStart : 1;

		double TickInterval = 0.05;



		//test custom attributes to see if we can start this task
		virtual bool CheckRequirements() { return true; };

		//locks resources that are needed for this task
		virtual bool CommitResources() { return true; };

		//unlock resources
		virtual void ReleaseResources() {};

		//task start behaviour
		virtual void TaskStart() {};

		//task tick behaviour
		virtual void TaskTick(double DeltaTime) {};

		//task end behaviour
		virtual void TaskEnd() {};

		void Complete();

		Task();
		Task(const Task&) = default;
	public:
		Task(Task&&) = default;

		inline size_t GetID()const noexcept { return TaskID.ID; }
		inline std::string_view GetName()const noexcept { return TaskName; }

		bool GetIsActive()const { return IsActive; };
		bool GetIsCompleted()const { return IsCompleted; };
		bool GetTickEnabled()const { return AllowTick; }
		bool CanTick()const { return IsActive&& CanEverTick&& AllowTick; }
		void SetTaskName(std::string_view newName);
		inline void SetTickEnabled(bool newTick) { AllowTick = newTick; };

		ETaskStartResponse Start(bool restart = false);

		void Cancel();

		Structures::TMultiDelegate<Task*, bool> OnActivationChanged;
		Structures::TMultiDelegate<Task*, ETaskEndResponse> OnTaskEnded;
		Structures::TMultiDelegate<Task*> OnTaskStarted;

		inline bool operator==(const Task& other)const { return TaskID == other.TaskID; }

		friend class TaskManager;
	};
}

namespace std
{
	template<> struct hash<AppComponents::FTaskID>
	{
		size_t operator()(const AppComponents::FTaskID& id)const
		{
			return id.ID;
		}
	};
}