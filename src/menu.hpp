#pragma once

#include <iostream>
#include <manymouse.h>
#include <map>
#include <math.h>
#include <sstream>
#include <string>

#include "gameapp.hpp"
#include "physics.hpp"
#include "physstruct.hpp"
#include "profiler.hpp"

namespace App
{
    void ClearConsole();
	void SetupMenu();
	void DrawMenu();
	int MenuFrame();
} // namespace App
