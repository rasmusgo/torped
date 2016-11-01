#ifndef INPUT_H
#define INPUT_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include "SDL.h"
#include <vector>
#include <string>
#include <map>
#include <python.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "profiler.h"
#include "action.h"

namespace App
{
    // From input.cpp
    void InitInput();
	int HandleGameEvents();
	void UpdateMice();
    void FlushMice();
	void ParseEvent(std::string event, float value, bool silent);
	void Bind(std::string event, Action *action);
	void UnBind(std::string event);

    extern bool discard_mouse_event;
    extern int num_mice;
    extern std::vector<float> mouse_sens;
    extern std::vector<SDL_Joystick *> joysticks;
    extern std::map<std::string, Action*> actions_table;

    // From gameapp.cpp
    #ifndef APPMODE
    #define APPMODE
    enum AppMode
    {
        MENU = 1,
        GAME
    };
    #endif

	void SwitchMode(AppMode mode);

    extern bool fatal_error;
    extern Profiler profiler;

} // namespace App
#endif // INPUT_H
