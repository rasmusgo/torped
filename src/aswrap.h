#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <angelscript.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

void AsMessageCallback(const asSMessageInfo *msg, void *param);
void AsWrapConsoleCmds(asIScriptEngine *as);
void AsRunFile(asIScriptEngine *as, const char *filename);
