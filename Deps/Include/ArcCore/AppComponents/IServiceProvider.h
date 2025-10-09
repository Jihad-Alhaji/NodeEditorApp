#pragma once
#include"IService.h"
#include<vector>
#include<memory>
namespace AppComponents
{
	class IServiceProvider
	{
		std::vector<std::shared_ptr<IService>> Services;
		bool bActive{ false };
	protected:
		//dispatches all services tick actions
		void ServicesTick(double DeltaTime);
		void SetvicesStart();
		void ServicesEnd();

		//initializes registered services
		void InitServices();

	public:
		void AddService(std::shared_ptr<IService> service);
		//default way to add services
		template<class service_t>
		std::shared_ptr<service_t> CreateService()
		{
			static_assert(std::is_base_of_v<IService, service_t>, "type must be of type IService!");
			std::shared_ptr<service_t> serv = std::make_shared<service_t>();
			AddService(serv);
			return serv;
		}

		std::string GetServicesNames()const;

		//activates the services provider
		inline void EnableServiceProvider() { bActive = true; }
		
		//deactivate the service provider
		inline void DisableServiceProvider() { bActive = false; }
		
		//is this service provider enabled
		inline bool IsServicesEnabled()const { return bActive; }

		//gets a service pointer if presents
		std::shared_ptr<IService> GetService(std::string_view servicename);
		bool RemoveService(std::string_view servicename);
	};
}

