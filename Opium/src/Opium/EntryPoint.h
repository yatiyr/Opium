#pragma once

// Entry points are currently the same for all platforms
// But this is going to be changed in the future
#ifdef OP_PLATFORM_WINDOWS

	extern Opium::Application* Opium::CreateApplication();

	int main(int argc, char** argv)
	{
		Opium::Logging::Init();
		OP_ENGINE_WARN("Initialized log for engine");
		OP_APP_INFO("Initialized app log!");

		auto app = Opium::CreateApplication();
		app->Run();
		delete app;
	}

#else

	extern Opium::Application* Opium::CreateApplication();

	int main(int argc, char** argv)
	{
		Opium::Logging::Init();

		auto app = Opium::CreateApplication();
		app->Run();
		delete app;
	}

#endif