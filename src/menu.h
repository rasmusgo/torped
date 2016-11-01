#ifndef MENU_H
#define MENU_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <math.h>
#include "manymouse/manymouse.h"
#include <angelscript.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "physstruct.h"
#include "physics.h"
#include "profiler.h"
#include "gameapp.h"

namespace App
{
    void ClearConsole();
	void SetupMenu();
	void DrawMenu();
	int MenuFrame();
} // namespace App
#endif // MENU_H
