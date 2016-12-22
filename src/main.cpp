//#include "SDL_main.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "console.h"
#include "gameapp.h"
#include "logging.h"

int main(int argc, char *argv[])
{
    loguru::add_file("logs/info.log", loguru::FileMode::Truncate, loguru::Verbosity_INFO);
    loguru::add_file("logs/full.log", loguru::FileMode::Truncate, loguru::Verbosity_MAX);
    loguru::add_callback("console", App::Console::LogHandler, &App::console, loguru::Verbosity_MAX);
    loguru::init(argc, argv); // Log time of start and set verbosity level.

	App::InitAll(argc, argv);
	atexit(App::QuitAll);

    while(App::DoFrame());

	return 0;
}
