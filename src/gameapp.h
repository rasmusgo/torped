#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <math.h>
#include <manymouse.h>
#include <angelscript.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "physstruct.h"
#include "physics.h"
#include "profiler.h"

namespace App
{
    // Types
    #ifndef APPMODE
    #define APPMODE
    enum AppMode
    {
        MENU = 1,
        GAME
    };
    #endif

    // Functions
    void InitAll(int argc, char *argv[]);
    void QuitAll();
	int DoFrame();
	void LoadWorld(const char filename[]);
    void SaveWorld(const char filename[]);
    void ClearWorld();
    void ReloadVideo();
	void SwitchMode(AppMode mode);
    void SaveScreenshot(const char filename[]);
	void SetupGame();
	void DrawGame();
	int GameFrame();
	void Quit();
	void DrawPhysics(PhyInstance &inst);
	void DrawHUD();
	//inline void DrawString(void *font, const char string[]);

    // Variables
	extern int xRes;
	extern int yRes;
	extern int fullscreen;
	// TODO: move world and others to better places
	extern AppMode appMode;

    extern int developermode;
    extern int wireframemode;
    extern int stereo3dmode;
    extern float stereo3d_depth;
    extern float stereo3d_focus;
    extern float view_depth;
    extern int profilermode;

    extern bool fatal_error;

    extern Profiler profiler;

    extern std::vector<SDL_Thread*> threads;

    extern asIScriptEngine *as_engine;
} // namespace App
