#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <math.h>
#include <thread>
#include <manymouse.h>

#include "physstruct.hpp"
#include "physics.hpp"
#include "profiler.hpp"

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

    extern std::vector<std::thread> threads;
} // namespace App
