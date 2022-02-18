#pragma once
#include <Opium/Core.h>
#include <Opium/Logging.h>
#include <EventSystem/AppEvent.h>

namespace Opium
{
	class OPIUM_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// Client will define this
	Application* CreateApplication();
}