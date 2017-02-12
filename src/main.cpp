//#include "SDL_main.h"

#include "console.hpp"
#include "gameapp.hpp"
#include "logging.hpp"

int main(int argc, char *argv[])
{
    GameApp gameapp(argc, argv);

    while(gameapp.DoFrame());

	return 0;
}
