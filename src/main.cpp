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

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <stdio.h>
FILE _iob[] = {*stdin, *stdout, *stderr};
extern "C" FILE * __cdecl __iob_func(void)
{
    return _iob;
}
#endif
