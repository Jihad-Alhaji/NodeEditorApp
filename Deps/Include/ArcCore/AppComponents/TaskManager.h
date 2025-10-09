#pragma once
#include<unordered_map>
#include"Task.h"
#include"IService.h"
#include<memory>
namespace AppComponents
{
	class TaskManager : public IService
	{
		void AddTask(std::shared_ptr<Task>&& task);
		std::vector<FTaskID> TasksPendingDestruction;
		std::unordered_map<size_t, std::shared_ptr<Task>> Tasks;


		void HandleTaskEnded(Task* task , ETaskEndResponse res);
		void HandleTaskStarted(Task* task);
		void HandleTaskActivationChanged(Task* task, bool res);

	public:
		static inline const char* TaskManagerServiceName = "Task Manager";
		ETaskStartResponse StartTask(FTaskID taskID, bool restart = false);
		void CancelTask(FTaskID taskID);
		std::shared_ptr<Task> GetTask(FTaskID taskID);

		//creates and adds a default task
		template<class TaskType>
		std::weak_ptr<TaskType> AddDefaultTask(FTaskID taskID)
		{
			static_assert(std::is_base_of_v<Task, TaskType>, "type must be of type Task!");
			//avoid id conflicts
			if (Tasks.contains(taskID))
				return std::weak_ptr<TaskType>{};

			auto task = std::make_shared<TaskType>();
			task->TaskID = taskID;
			std::weak_ptr<TaskType> res = task;
			AddTask(std::move(task));
			return res;
		}

		void Tick(double deltatime)override;
		void Start() override;
		void End() override;
		void Init() override;

		Structures::TMultiDelegate<Task*, bool> OnTaskActivationChanged;
		Structures::TMultiDelegate<Task*, ETaskEndResponse> OnTaskEnded;
		Structures::TMultiDelegate<Task*> OnTaskStarted;
	};
}