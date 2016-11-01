//#include "SDL_main.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "gameapp.h"

int main(int argc, char *argv[])
{
	App::InitAll(argc, argv);
	atexit(App::QuitAll);

    while(App::DoFrame());

	return 0;
}
