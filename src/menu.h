#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <iostream>
#include <manymouse.h>
#include <map>
#include <math.h>
#include <sstream>
#include <string>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "gameapp.h"
#include "physics.h"
#include "physstruct.h"
#include "profiler.h"

namespace App
{
    void ClearConsole();
	void SetupMenu();
	void DrawMenu();
	int MenuFrame();
} // namespace App
