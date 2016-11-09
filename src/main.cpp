//#include "SDL_main.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "console.h"
#include "gameapp.h"
#include "logging.h"

int main(int argc, char *argv[])
{
    loguru::add_callback("console", App::Console::LogHandler, &App::console, loguru::Verbosity_MAX);
    loguru::init(argc, argv);

	App::InitAll(argc, argv);
	atexit(App::QuitAll);

    while(App::DoFrame());

	return 0;
}
